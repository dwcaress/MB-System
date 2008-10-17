/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust_prog.c	3/23/00
 *    $Id: mbnavadjust_prog.c,v 5.34 2008-10-17 07:52:44 caress Exp $
 *
 *    Copyright (c) 2000-2008 by
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
 * $Log: not supported by cvs2svn $
 * Revision 5.33  2008/09/27 03:27:10  caress
 * Working towards release 5.1.1beta24
 *
 * Revision 5.32  2008/09/11 20:12:43  caress
 * Checking in updates made during cruise AT15-36.
 *
 * Revision 5.31  2008/07/10 18:08:10  caress
 * Proceeding towards 5.1.1beta20.
 *
 * Revision 5.28  2008/05/16 22:42:32  caress
 * Release 5.1.1beta18 - working towards use of 3D uncertainty.
 *
 * Revision 5.27  2007/11/16 17:54:10  caress
 * Changes as of 11/16/2007
 *
 * Revision 5.26  2007/10/08 16:02:46  caress
 * MBnavadjust now performs an initial inversion for the average offsets for each independent block of data and then removes that average signal before performing the full inversion.
 *
 * Revision 5.25  2007/05/14 06:34:11  caress
 * Many changes to mbnavadjust, including adding z offsets and 3D search grids.
 *
 * Revision 5.24  2006/12/15 21:42:49  caress
 * Incremental CVS update.
 *
 * Revision 5.23  2006/11/10 22:36:05  caress
 * Working towards release 5.1.0
 *
 * Revision 5.22  2006/08/09 22:41:27  caress
 * Fixed programs that read or write grids so that they do not use the GMT_begin() function; these programs will now work when GMT is built in the default fashion, when GMT is built in the default fashion, with "advisory file locking" enabled.
 *
 * Revision 5.21  2006/07/27 18:42:52  caress
 * Working towards 5.1.0
 *
 * Revision 5.20  2006/06/16 19:30:58  caress
 * Check in after the Santa Monica Basin Mapping AUV Expedition.
 *
 * Revision 5.19  2006/01/24 19:18:42  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.18  2006/01/06 18:25:21  caress
 * Working towards 5.0.8
 *
 * Revision 5.17  2005/11/05 00:57:03  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.16  2005/06/04 04:34:07  caress
 * Added notion of "truecrossings", so it's possible to process the data while only looking at crossing tracks and ignoring overlap points.
 *
 * Revision 5.15  2004/12/18 01:35:42  caress
 * Working towards release 5.0.6.
 *
 * Revision 5.14  2004/12/02 06:34:27  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.13  2004/05/21 23:31:27  caress
 * Moved to new version of BX GUI builder
 *
 * Revision 5.12  2003/04/17 21:07:49  caress
 * Release 5.0.beta30
 *
 * Revision 5.11  2002/08/28 01:32:45  caress
 * Finished first cut at man page.
 *
 * Revision 5.10  2002/03/26 07:43:57  caress
 * Release 5.0.beta15
 *
 * Revision 5.9  2001/10/19 00:55:42  caress
 * Now tries to use relative paths.
 *
 * Revision 5.8  2001/08/04  01:02:24  caress
 * Fixed small bugs.
 *
 * Revision 5.7  2001/08/02  01:48:30  caress
 * Fixed call to mb_pr_get_heading() and mb_pr_get_rollbias().
 *
 * Revision 5.6  2001/07/20  00:33:43  caress
 * Release 5.0.beta03
 *
 * Revision 5.5  2001/06/03 07:05:54  caress
 * Release 5.0.beta01.
 *
 * Revision 5.4  2001/03/22 21:09:11  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.3  2001/01/22  07:45:59  caress
 * Version 5.0.beta01
 *
 * Revision 5.2  2000/12/21  00:44:15  caress
 * Changed decimation from import parameter to contouring/gridding parameter.
 *
 * Revision 5.1  2000/12/10  20:29:34  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:55:48  caress
 * First cut at Version 5.0.
 *
 * Revision 4.1  2000/10/03  21:49:28  caress
 * Fixed handling count of nav points while importing data.
 *
 * Revision 4.0  2000/09/30  07:00:06  caress
 * Snapshot for Dale.
 *
 *
 *
 */

/*--------------------------------------------------------------------*/

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

/* MBIO include files */
#include "mb_format.h"
#include "mb_status.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_contour.h"
#include "mbsys_ldeoih.h"

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
static char rcs_id[] = "$Id: mbnavadjust_prog.c,v 5.34 2008-10-17 07:52:44 caress Exp $";
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

/* color control values */
#define	WHITE	0	
#define	BLACK	1	
#define RED	2
#define GREEN	3
#define BLUE	4
#define CORAL	5
#define	XG_SOLIDLINE	0
#define	XG_DASHLINE	1
int	pcont_xgid;
int	pcorr_xgid;
int	pzoff_xgid;
int	pmodp_xgid;
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

/* minimum initial sigma_crossing (meters) */
#define	SIGMA_MINIMUM	0.1;

/* system function declarations */
char	*ctime();
char	*getenv();

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
	project.num_snavs = 0;
	project.num_pings = 0;
	project.num_beams = 0;
	project.num_crossings = 0;
	project.num_crossings_good = 0;
	project.num_crossings_alloc = 0;
 	project.num_crossings_analyzed = 0;
	project.num_goodcrossings = 0;
	project.num_truecrossings = 0;
	project.num_truecrossings_good = 0;
 	project.num_truecrossings_analyzed = 0;
 	project.crossings = NULL;
	project.num_ties = 0;
	project.inversion = MBNA_INVERSION_NONE;
	project.modelplot = MB_NO;
	project.logfp = NULL;
 	mbna_status = MBNA_STATUS_GUI;
 	mbna_view_list = MBNA_VIEW_LIST_FILES;
 	project.section_length = 10.0;
 	project.section_soundings = 20000;
 	project.decimation = 1;
	project.precision = SIGMA_MINIMUM;
	project.zoffsetwidth = 5.0;
	mbna_file_id_1 = -1;
	mbna_section_1 = -1;
	mbna_file_id_2 = -1;
	mbna_section_2 = -1;
 	mbna_current_crossing = -1;
 	mbna_current_tie = -1;
	mbna_naverr_load = MB_NO;
 	mbna_file_select = MBNA_SELECT_NONE;
	mbna_crossing_select = MBNA_SELECT_NONE;
	mbna_tie_select = MBNA_SELECT_NONE;
	project.cont_int = 25.;
	project.col_int = 100.;
	project.tick_int = 100.;
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
	mbna_smoothweight = 10.0;
	mbna_offsetweight = 1.0;
	mbna_misfit_center = MBNA_MISFIT_AUTOCENTER;
	mbna_minmisfit_nthreshold = MBNA_MISFIT_NTHRESHOLD;
	mbna_minmisfit = 0.0;
	mbna_bias_mode = MBNA_BIAS_SAME;
	mbna_allow_set_tie = MB_NO;
	mbna_modelplot_zoom_x1 = 0;
	mbna_modelplot_zoom_x2 = 0;
	mbna_reset_crossings = MB_NO;

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
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhI:i:Rr")) != -1)
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
int mbnavadjust_set_graphics(int cn_xgid, int cr_xgid, int zc_xgid)
{
	/* local variables */
	char	*function_name = "mbnavadjust_set_graphics";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       cn_xgid:      %d\n",cn_xgid);
		fprintf(stderr,"dbg2       cr_xgid:      %d\n",cr_xgid);
		fprintf(stderr,"dbg2       zc_xgid:      %d\n",zc_xgid);
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
				project.num_crossings_good = 0;
				project.num_crossings_alloc = 0;
				project.num_crossings_analyzed = 0;
				project.num_goodcrossings = 0;
				project.num_truecrossings = 0;
				project.num_truecrossings_good = 0;
				project.num_truecrossings_analyzed = 0;
				project.crossings = NULL;
				project.num_ties = 0;
				project.inversion = MBNA_INVERSION_NONE;
				project.precision = SIGMA_MINIMUM;
				project.zoffsetwidth = 5.0;
				
				/* create data directory */
				if (mkdir(project.datadir,00775) != 0)
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
	FILE	*hfp;
	struct mbna_file *file;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	crossingstatus;
	char	datalist[STRING_MAX];
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
		fprintf(hfp,"##MBNAVADJUST PROJECT\n");
		fprintf(hfp,"MB-SYSTEM_VERSION\t%s\n",MB_VERSION);
		fprintf(hfp,"PROGRAM_VERSION\t%s\n",rcs_id);
		fprintf(hfp,"FILE_VERSION\t2.00\n");
		fprintf(hfp,"NAME\t%s\n",project.name);
		fprintf(hfp,"PATH\t%s\n",project.path);
		fprintf(hfp,"HOME\t%s\n",project.home);
		fprintf(hfp,"DATADIR\t%s\n",project.datadir);
		fprintf(hfp,"NUMFILES\t%d\n",project.num_files);
		fprintf(hfp,"NUMCROSSINGS\t%d\n",project.num_crossings);
		fprintf(hfp,"SECTIONLENGTH\t%f\n",project.section_length);
		fprintf(hfp,"SECTIONSOUNDINGS\t%d\n",project.section_soundings);
		fprintf(hfp,"DECIMATION\t%d\n",project.decimation);
		fprintf(hfp,"CONTOURINTERVAL\t%f\n",project.cont_int);
		fprintf(hfp,"COLORINTERVAL\t%f\n",project.col_int);
		fprintf(hfp,"TICKINTERVAL\t%f\n",project.tick_int);
		fprintf(hfp,"INVERSION\t%d\n",project.inversion);
		fprintf(hfp,"PRECISION\t%f\n",project.precision);
		fprintf(hfp,"ZOFFSETWIDTH\t%f\n",project.zoffsetwidth);
		for (i=0;i<project.num_files;i++)
			{
			/* write out basic file info */
			file = &project.files[i];
			fprintf(hfp,"FILE %4d %4d %4d %4d %4.1f %4.1f %4.1f %4.1f %4d %4d %s\n",
				i,
				file->status,
				file->id,
				file->format,
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
				fprintf(hfp,"SECTION %4d %5d %5d %d %d %10.6f %16.6f %16.6f %12.7f %12.7f %12.7f %12.7f %9.3f %9.3f %d\n",
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
				    fprintf(hfp,"SNAV %4d %5d %10.6f %16.6f %12.7f %12.7f %12.7f %12.7f %12.7f\n",
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
				fprintf(hfp,"TIE %5d %5d %12.7f %5d %12.7f %12.7f %12.7f %12.7f %1.1d %12.7f %12.7f %12.7f\n",
					j,
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
				fprintf(hfp,"COV %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f\n",
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

	/* open and write datalist file */
	sprintf(datalist,"%s%s.mb-1",project.path,project.name);
	if ((hfp = fopen(datalist,"w")) != NULL)
		{
		for (i=0;i<project.num_files;i++)
			{
			/* write file entry */
			file = &project.files[i];
			fprintf(hfp,"%s %d\n", file->file, file->format);
			}
		}
	
	/* else set error */
	else
		{
		status = MB_FAILURE;
		sprintf(message,"Unable to update project %s\n > Datalist file: %s\n", 
			project.name, datalist);
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
	char	*result;
	int	versionmajor, versionminor;
	int	nscan, idummy;
	double	lonmin, lonmax, latmin, latmax;
	int	nblock;
	int	i, j, k, l;

	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* open and read home file */
	status = MB_SUCCESS;
	if ((hfp = fopen(project.home,"r")) != NULL)
		{
		/* check for proper header */
		if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
			|| strncmp(buffer,"##MBNAVADJUST PROJECT",21) != 0)
			status = MB_FAILURE;
			
		/* read basic names and stats */
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
			    	|| strcmp(label,"MB-SYSTEM_VERSION") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
				|| strcmp(label,"PROGRAM_VERSION") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d.%d",label,&versionmajor,&versionminor)) != 3
				|| strcmp(label,"FILE_VERSION") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
				|| strcmp(label,"NAME") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
				|| strcmp(label,"PATH") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
				|| strcmp(label,"HOME") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
				|| strcmp(label,"DATADIR") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d",label,&project.num_files)) != 2
				|| strcmp(label,"NUMFILES") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d",label,&project.num_crossings)) != 2
				|| strcmp(label,"NUMCROSSINGS") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %lf",label,&project.section_length)) != 2
				|| strcmp(label,"SECTIONLENGTH") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((100*versionmajor + versionminor) > 100)
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d",label,&project.section_soundings)) != 2
				|| strcmp(label,"SECTIONSOUNDINGS") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d",label,&project.decimation)) != 2
				|| strcmp(label,"DECIMATION") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %lf",label,&project.cont_int)) != 2
				|| strcmp(label,"CONTOURINTERVAL") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %lf",label,&project.col_int)) != 2
				|| strcmp(label,"COLORINTERVAL") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %lf",label,&project.tick_int)) != 2
				|| strcmp(label,"TICKINTERVAL") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d",label,&project.inversion)) != 2
				|| strcmp(label,"INVERSION") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS)
			{
			if (versionmajor > 1 || versionminor > 2)
				{
				if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
					|| (nscan = sscanf(buffer,"%s %lf",label,&project.precision)) != 2
					|| strcmp(label,"PRECISION") != 0)
				status = MB_FAILURE;
				}
			else
				project.precision = SIGMA_MINIMUM;
			}
		if (status == MB_SUCCESS)
			{
			if ((versionmajor > 1 || versionminor > 4)
				&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
					|| (nscan = sscanf(buffer,"%s %lf",label,&project.zoffsetwidth)) != 2
					|| strcmp(label,"ZOFFSETWIDTH") != 0))
				status = MB_FAILURE;
			else
				project.zoffsetwidth = 5.0;
			}

		/* allocate memory for files array */
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
		for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			file->num_sections_alloc = 0;
			file->sections = NULL;
			file->num_snavs = 0;
			file->num_pings = 0;
			file->num_beams = 0;
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
		nblock = 0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    if (i==0 || file->sections[0].continuity == MB_NO)
		    	{
			nblock++;
			}
		    file->block = nblock - 1;
		    file->block_offset_x = 0.0;
		    file->block_offset_y = 0.0;
		    file->block_offset_z = 0.0;
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
				&& ((100*versionmajor + versionminor) >= 106))
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
				&& ((100*versionmajor + versionminor) >= 102))
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
				
			/* read ties */
			if (status == MB_SUCCESS)
			for (j=0;j<crossing->num_ties;j++)
				{
				/* read each tie */
				tie = &crossing->ties[j];
				if (status == MB_SUCCESS && ((100*versionmajor + versionminor) > 103))
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
					tie->offset_z_m = 0.0;
					tie->inversion_offset_z_m = 0.0;
					}
					
				/* for version 2.0 or later read covariance */
				if (status == MB_SUCCESS && ((100*versionmajor + versionminor) >= 200))
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
						tie->sigmax2[0] = 1.0;
						tie->sigmax2[1] = 0.0;
						tie->sigmax2[2] = 0.0;
						mbna_minmisfit_sr2 = 1.0;
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
			
		/* get crossing overlap values if not already set */
		if (project.open == MB_YES)
			{
			for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);
				if (crossing->overlap <= 0)
					{
					mbnavadjust_crossing_overlap(i, NULL, NULL, NULL, NULL);
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
	int	done;
	char	file[STRING_MAX];
	double	weight;
	int	form;

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
	while (done == MB_NO)
		{
		if (iformat > 0)
			{
			status = mbnavadjust_import_file(path,iformat);
			done = MB_YES;
			}
		else if (iformat == -1)
			{
			if (status = mb_datalist_open(mbna_verbose,&datalist,
							path,MB_DATALIST_LOOK_NO,&error) == MB_SUCCESS)
				{
				while (done == MB_NO)
					{
					if (status = mb_datalist_read(mbna_verbose,datalist,
							file,&form,&weight,&error)
							== MB_SUCCESS)
						{
						status = mbnavadjust_import_file(file,form);
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
int mbnavadjust_import_file(char *path, int iformat)
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

	int	output_id, found;
	int	obeams_bath,obeams_amp,opixels_ss;
	int	iform;
	double	good_depth;
	int	nread, first;
	int	output_open = MB_NO;
	int	good_bath, good_beams, new_segment;
	int	disqualify;
	double	headingx, headingy, mtodeglon, mtodeglat;
	double	lon, lat;
	double	navlon_old, navlat_old;
	FILE	*nfp;
	struct mbna_file *file, *cfile;
	struct mbna_crossing *crossing;
	struct mbna_section *section, *csection;
	struct mbsys_ldeoih_struct *ostore;
	struct mb_io_struct *omb_io_ptr;
	int	new_pings, new_crossings;
	int	overlap;
	double	dx1, dy1, dx2, dy2;
	double	lon1min, lon1max, lat1min, lat1max;
	double	lon2min, lon2max, lat2min, lat2max;
	int	mbp_heading_mode;
	double	mbp_headingbias;
	int	mbp_rollbias_mode;
	double	mbp_rollbias;
	double	mbp_rollbias_port;
	double	mbp_rollbias_stbd;
	double	depthmax, distmax, depthscale, distscale;
	double	lonmin, lonmax, latmin, latmax;
 	int	i, j, k;
	int	ii1, jj1, kk1, ii2, jj2, kk2;
	

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
	output_open = MB_NO;
	project.inversion = MBNA_INVERSION_NONE;
	new_pings = 0;
	new_crossings = 0;
		
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
				
			/* check for good bathymetry */
			good_bath = MB_NO;
			if (kind == MB_DATA_DATA 
				&& error == MB_ERROR_NO_ERROR)
				{
				for (i=0;i<beams_bath;i++)
					{
					if (mb_beam_ok(beamflag[i]) && bath[i] != 0.0)
						{
						good_bath = MB_YES;
						good_depth = bath[i];
						}
					}
				}
				
			/* deal with new file */
			if (good_bath == MB_YES && first == MB_YES)
				{
				file = &project.files[project.num_files];
				file->status = MBNA_FILE_OK;
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
			else if (good_bath == MB_YES
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
			if (good_bath == MB_YES && new_segment == MB_YES)
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
				else if (project.num_files > 1)
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
				section->depthmin = good_depth;
				section->depthmax = good_depth;
				section->contoursuptodate = MB_NO;
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
			if (good_bath == MB_YES
				&& section->num_pings > 1)
				section->distance += distance;
				
			/* handle good bathymetry */
			if (good_bath == MB_YES)
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
						good_bath = MB_YES;
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
						section->depthmin = MIN(section->depthmin,bath[i]);
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
					ostore->depth_scale = 1000 * depthscale + 1;
					depthscale = 0.001 * ostore->depth_scale;
					ostore->distance_scale = 1000 * distscale + 1;
					distscale = 0.001 * ostore->distance_scale;
					ostore->transducer_depth = draft / depthscale;
					
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
				/*fprintf(stderr, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f\r\n",
					time_i[0], time_i[1], time_i[2], time_i[3],
					time_i[4], time_i[5], time_i[6], time_d,
					navlon, navlat, heading, speed);
				fprintf(nfp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f\r\n",
					time_i[0], time_i[1], time_i[2], time_i[3],
					time_i[4], time_i[5], time_i[6], time_d,
					navlon, navlat, heading, speed);*/
				/*fprintf(stderr, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						time_i[0], time_i[1], time_i[2], time_i[3],
						time_i[4], time_i[5], time_i[6], time_d,
						navlon, navlat, heading, speed, 
						draft, roll, pitch, heave);*/
				fprintf(nfp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
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
				fprintf(stderr,"dbg2       navlon:         %f\n",navlon);
				fprintf(stderr,"dbg2       navlat:         %f\n",navlat);
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
		
		/* now search for crossings */
		if (file != NULL && first != MB_YES)
			{
			for (k=0;k<file->num_sections;k++)
				{
				/* first get coverage mask */
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
				
				/* now compare coverage masks */
				for (i=0;i<project.num_files;i++)
				    {
				    cfile = (struct mbna_file *) &project.files[i];
				    for (j=0;j<cfile->num_sections;j++)
					{
					csection = (struct mbna_section *) &cfile->sections[j];
					dx2 = (csection->lonmax - csection->lonmin) / MBNA_MASK_DIM;
					dy2 = (csection->latmax - csection->latmin) / MBNA_MASK_DIM;
					disqualify = MB_NO;
					if (i == project.num_files - 1
						&& j >= k)
						disqualify = MB_YES;
					else if (i == project.num_files - 1
						&& j == k - 1
						&& section->continuity == MB_YES)
						disqualify = MB_YES;
					else if (i == project.num_files - 2
						&& k == 0
						&& j == cfile->num_sections - 1
						&& section->continuity == MB_YES)
						disqualify = MB_YES;
					if (disqualify == MB_NO
						&& (section->lonmin < csection->lonmax)
						&& (section->lonmax > csection->lonmin)
						&& (section->latmin < csection->latmax)
						&& (section->latmax > csection->latmin))
					    {
					    overlap = 0;
					    for (ii1=0;ii1<MBNA_MASK_DIM;ii1++)
					    for (jj1=0;jj1<MBNA_MASK_DIM;jj1++)
						{
						kk1 = ii1 + jj1 * MBNA_MASK_DIM;
						if (section->coverage[kk1] == 1)
						    {
						    lon1min = section->lonmin + dx1 * ii1;
						    lon1max = section->lonmin + dx1 * (ii1 + 1);
						    lat1min = section->latmin + dy1 * jj1;
						    lat1max = section->latmin + dy1 * (jj1 + 1);
						    for (ii2=0;ii2<MBNA_MASK_DIM;ii2++)
						    for (jj2=0;jj2<MBNA_MASK_DIM;jj2++)
							{
							kk2 = ii2 + jj2 * MBNA_MASK_DIM;
							if (section->coverage[kk2] == 1)
							    {
							    lon2min = csection->lonmin + dx2 * ii2;
							    lon2max = csection->lonmin + dx2 * (ii2 + 1);
							    lat2min = csection->latmin + dy2 * jj2;
							    lat2max = csection->latmin + dy2 * (jj2 + 1);
							    if ((lon1min < lon2max)
								&& (lon1max > lon2min)
								&& (lat1min < lat2max)
								&& (lat1max > lat2min))
								overlap++;
							    }
							}
						    }
						}
					    if (overlap == 0)
						disqualify = MB_YES;
					    }
					else
					    disqualify = MB_YES;
						
					if (disqualify == MB_NO)
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
					    crossing->overlap = overlap;
					    crossing->file_id_1 = file->id;
					    crossing->section_1 = k;
					    crossing->file_id_2 = cfile->id;
					    crossing->section_2 = j;
					    crossing->num_ties = 0;
					    project.num_crossings++;
					    new_crossings++;
						
					    /* check if this is a true crossing */
					    if (mbnavadjust_sections_intersect(project.num_crossings-1) == MB_YES)
					    	{
						crossing->truecrossing = MB_YES;
						project.num_truecrossings++;
						}
						
					    /* recalculate crossing overlap */
					    mbnavadjust_crossing_overlap(project.num_crossings-1,
					    				NULL, NULL, NULL, NULL);
					    if (crossing->overlap >= 25)
					    	project.num_goodcrossings++;
/*fprintf(stderr,"added crossing:%d  %4d %4d   %4d %4d\n",
project.num_crossings-1,
crossing->file_id_1,crossing->section_1,
crossing->file_id_2,crossing->section_2);*/
					    }
					}
				    }
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
int mbnavadjust_fix_file()
{
	/* local variables */
	char	*function_name = "mbnavadjust_fix_file";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
 	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}
		
     	/* fix selected file */
    	if (project.open == MB_YES
    		&& project.num_files > 0
		&& mbna_file_select >= 0
		&& mbna_file_select < project.num_files
		&& project.files[mbna_file_select].status 
		    == MBNA_FILE_OK)
		{
		/* fix file */
		project.files[mbna_file_select].status = MBNA_FILE_FIXED;
		if (project.inversion == MBNA_INVERSION_CURRENT)
			project.inversion = MBNA_INVERSION_OLD;
			
		/* now set all all crossings between this
		   file and other fixed files to skip status */
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &project.crossings[i];
		    if (crossing->status != MBNA_CROSSING_STATUS_SKIP
			&& ((crossing->file_id_1 == mbna_file_select
				&& project.files[crossing->file_id_2].status 
				    == MBNA_FILE_FIXED)
			    || (crossing->file_id_2 == mbna_file_select
				&& project.files[crossing->file_id_1].status 
				    == MBNA_FILE_FIXED)))
			{
			if (crossing->status == MBNA_CROSSING_STATUS_NONE)
			    	{
				project.num_crossings_analyzed++;
				if (crossing->truecrossing == MB_YES)
 					project.num_truecrossings_analyzed++;
				}
			crossing->status = MBNA_CROSSING_STATUS_SKIP;
			}
		    }
			
		/* write out updated project */
		mbnavadjust_write_project();
		
		/* add info text */
		sprintf(message, "Set file %d fixed: %s\n",
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
int mbnavadjust_unfix_file()
{
	/* local variables */
	char	*function_name = "mbnavadjust_unfix_file";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
 	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}
		
     	/* unfix selected file */
    	if (project.open == MB_YES
    		&& project.num_files > 0
		&& mbna_file_select >= 0
		&& mbna_file_select < project.num_files
		&& project.files[mbna_file_select].status 
		    == MBNA_FILE_OK)
		{
		/* unfix file */
		project.files[mbna_file_select].status = MBNA_FILE_OK;
		if (project.inversion == MBNA_INVERSION_CURRENT)
			project.inversion = MBNA_INVERSION_OLD;
			
		/* now set all all crossings between this
		   file and other fixed files to none status */
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &project.crossings[i];
		    if (crossing->status == MBNA_CROSSING_STATUS_SKIP
			&& ((crossing->file_id_1 == mbna_file_select
				&& project.files[crossing->file_id_2].status 
				    == MBNA_FILE_FIXED)
			    || (crossing->file_id_2 == mbna_file_select
				&& project.files[crossing->file_id_1].status 
				    == MBNA_FILE_FIXED)))
			{
			project.num_crossings_analyzed--;
			crossing->status = MBNA_CROSSING_STATUS_NONE;
			}
		    }
			
		/* write out updated project */
		mbnavadjust_write_project();
		
		/* add info text */
		sprintf(message, "Set file %d unfixed: %s\n",
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
/*fprintf(stderr, "tie %d of crossing %d saved...\n", mbna_current_tie, mbna_current_crossing);*/
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
			    mbna_offset_z = tie->offset_y_m;
			    }
			else
			    {
			    mbna_current_tie = -1;
			    }
  			}
  			
  		/* load the crossing */
  		if (mbna_current_crossing >= 0)
  			{
  			mbnavadjust_crossing_load();
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
	int	i, j, k;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}
		
     	/* else get current good crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS)
    		{
    		/* get next good crossing */
		j = -1;
		k = -1;
		for (i=0;i<project.num_crossings;i++)
			{
			if (project.crossings[i].overlap >= MBNA_OVERLAP_THRESHOLD)
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
		
     	/* else get current true crossing */
    	else if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS)
    		{
    		/* get next true crossing */
		j = -1;
		k = -1;
		for (i=0;i<project.num_crossings;i++)
			{
			if (project.crossings[i].truecrossing == MB_YES)
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
		
     	/* else get current tie */
    	else if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& mbna_view_list == MBNA_VIEW_LIST_TIES)
    		{
    		/* get next tie */
    		crossing = &project.crossings[mbna_current_crossing];
		
		/* if another tie to current crossing use it */
		if (crossing->num_ties > 0 && mbna_current_tie < crossing->num_ties - 1)
			mbna_current_tie++;
			
		/* else loop over crossings looking for next valid tie */
		else 
			{
			j = -1;
			k = -1;
			for (i=0;i<project.num_crossings;i++)
				{
				if (project.crossings[i].num_ties > 0)
					{
					if (j < 0)
						j = i;
					if (k < 0 && i > mbna_current_crossing)
						k = i;
					}
				}
			if (k >= 0)
				{
				mbna_current_crossing = k;
				mbna_current_tie = 0;
				}
			else if (j >= 0)
				{
				mbna_current_crossing = j;
				mbna_current_tie = 0;
				}
			else
				{
				mbna_current_crossing = 0;
				mbna_current_tie = -1;
				}
			}
   		}
 		
     	/* get current crossing */
    	else if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
    		/* get next crossing */
    		if (mbna_current_crossing >= project.num_crossings - 1)
    			mbna_current_crossing = 0;
    		else
    		 	mbna_current_crossing++;
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
		    }
		else
		    {
		    mbna_current_tie = -1;
		    }
  		}

  	/* load the crossing */
  	if (mbna_current_crossing >= 0)
  		{
  		mbnavadjust_crossing_load();
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
	int	i, j, k;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}
		
     	/* get current crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& mbna_view_list != MBNA_VIEW_LIST_TRUECROSSINGS)
    		{
    		/* get previous crossing */
    		if (mbna_current_crossing <= 0)
    			mbna_current_crossing = project.num_crossings - 1;
    		else
    		 	mbna_current_crossing--;
    		 	
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
			    }
			else
			    {
			    mbna_current_tie = -1;
			    }
  			}
  			
  		/* load the crossing */
  		if (mbna_current_crossing >= 0)
  			{
  			mbnavadjust_crossing_load();
  			}
   		}
		
     	/* else get current true crossing */
    	else if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS)
    		{
    		/* get previous true crossing */
    		if (mbna_current_crossing <= 0)
    			mbna_current_crossing = project.num_crossings - 1;
    		else
    		 	mbna_current_crossing--;

   		/* get previous true crossing */
		j = -1;
		k = -1;
		for (i=0;i<project.num_crossings;i++)
			{
			if (project.crossings[i].truecrossing == MB_YES)
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
			    }
			else
			    {
			    mbna_current_tie = -1;
			    }
  			}
  			
  		/* load the crossing */
  		if (mbna_current_crossing >= 0)
  			{
  			mbnavadjust_crossing_load();
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
int mbnavadjust_naverr_nextunset()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_nextunset";
	int	status = MB_SUCCESS;
	int	start_crossing;
	int	found;
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
    		/* get next unset crossing */
    		found = MB_NO;
    		start_crossing = mbna_current_crossing + 1;
    		if (start_crossing >= project.num_crossings - 1)
    			start_crossing = 0;
    		for (i=start_crossing;
    			i<project.num_crossings && found == MB_NO;
    			i++)
    			{
    			crossing = &project.crossings[i];
    			if (crossing->status == MBNA_CROSSING_STATUS_NONE
				&& ((mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS
						&& crossing->truecrossing == MB_YES)
					|| (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS
						&& crossing->overlap >= MBNA_OVERLAP_THRESHOLD)
					|| (mbna_view_list != MBNA_VIEW_LIST_GOODCROSSINGS
						&& mbna_view_list != MBNA_VIEW_LIST_TRUECROSSINGS)))
    				{
    				mbna_current_crossing = i;
    				found = MB_YES;
    				}
    			}
    		if (found == MB_NO)
    			{
    			for (i=0;
    				i<start_crossing && found == MB_NO;
    				i++)
    				{
    				crossing = &project.crossings[i];
    				if (crossing->status == MBNA_CROSSING_STATUS_NONE
					&& ((mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS
						&& crossing->truecrossing == MB_YES)
					|| (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS
						&& crossing->overlap >= MBNA_OVERLAP_THRESHOLD)
					|| (mbna_view_list != MBNA_VIEW_LIST_GOODCROSSINGS
						&& mbna_view_list != MBNA_VIEW_LIST_TRUECROSSINGS)))
    					{
    					mbna_current_crossing = i;
    					found = MB_YES;
    					}
    				}
    			}
    		if (found == MB_NO && mbna_current_crossing < 0)
    			mbna_current_crossing = 0;
		else if (found == MB_NO)
			{
			if (mbna_current_crossing >= project.num_crossings - 1)
				mbna_current_crossing = 0;
			else
				mbna_current_crossing++;
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
			    }
			else
			    {
			    mbna_current_tie = -1;
			    }
  			}
  			
  		/* load the crossing */
  		if (mbna_current_crossing >= 0)
  			{
  			mbnavadjust_crossing_load();
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
    			tie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
    			tie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
    			tie->offset_z_m = mbna_offset_z;
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
					fprintf(stderr,"dbg2       section1->snav_lon[%d]:    %f\n",i,section1->snav_lon[i]);
					fprintf(stderr,"dbg2       section1->snav_lat[%d]:    %f\n",i,section1->snav_lat[i]);
					}
				fprintf(stderr,"dbg2       mbna_snav_2:        %d\n",mbna_snav_2);
				fprintf(stderr,"dbg2       mbna_snav_2_time_d: %f\n",mbna_snav_2_time_d);
				fprintf(stderr,"dbg2       mbna_snav_2_lon:    %f\n",mbna_snav_2_lon);
				fprintf(stderr,"dbg2       mbna_snav_2_lat:    %f\n",mbna_snav_2_lat);
				fprintf(stderr,"dbg2       section2->num_snav:  %d\n",section2->num_snav);
				for (i=0;i<section2->num_snav;i++)
					{
					fprintf(stderr,"dbg2       section2->snav_time_d[%d]: %f\n",i,section2->snav_time_d[i]);
					fprintf(stderr,"dbg2       section2->snav_lon[%d]:    %f\n",i,section2->snav_lon[i]);
					fprintf(stderr,"dbg2       section2->snav_lat[%d]:    %f\n",i,section2->snav_lat[i]);
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
		}
		
     	/* get current crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
     		/* retrieve crossing parameters */
    		if (mbna_current_crossing >= 0 
		    && mbna_current_tie >= 0)
    			{
			/* add info text */
    			crossing = &project.crossings[mbna_current_crossing];
    			tie = &crossing->ties[mbna_current_tie];
			sprintf(message,"Delete Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
				mbna_current_tie, mbna_current_crossing,
				crossing->file_id_1, crossing->section_1, tie->snav_1,
				crossing->file_id_2, crossing->section_2, tie->snav_2,
				tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
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
    			tie = &crossing->ties[mbna_current_tie];
			if (crossing->num_ties <= 0)
			    {
			    crossing->num_ties = 0;
    			    crossing->status = MBNA_CROSSING_STATUS_SKIP;
			    }
     			mbna_snav_1 = tie->snav_1;
     			mbna_snav_1_time_d = tie->snav_1_time_d;
			mbna_snav_2 = tie->snav_2;
     			mbna_snav_2_time_d = tie->snav_2_time_d;
			mbna_offset_x = tie->offset_x;
			mbna_offset_y = tie->offset_y;
			mbna_offset_z = tie->offset_z_m;
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
   			if (project.inversion == MBNA_INVERSION_CURRENT)
    				project.inversion = MBNA_INVERSION_OLD;
		
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
    	if ((mbna_status == MBNA_STATUS_NAVERR || mbna_status == MBNA_STATUS_MAKECONTOUR)
		&& project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
    		{
		/* put up message */
		sprintf(message,"Loading crossing %d...",mbna_current_crossing);
		do_message_on(message);
		
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
		    }
		else
		    {
		    mbna_offset_x = 0.0;
		    mbna_offset_y = 0.0;
		    mbna_offset_z = 0.0;
		    }
		mbna_lon_min = MIN(section1->lonmin,section2->lonmin);
		mbna_lon_max = MAX(section1->lonmax,section2->lonmax);
		mbna_lat_min = MIN(section1->latmin,section2->latmin);
		mbna_lat_max = MAX(section1->latmax,section2->latmax);
		mbna_plot_lon_min = mbna_lon_min;
		mbna_plot_lon_max = mbna_lon_max;
		mbna_plot_lat_min = mbna_lat_min;
		mbna_plot_lat_max = mbna_lat_max;
		mb_coor_scale(mbna_verbose,0.5 * (mbna_lat_min + mbna_lat_max),
				&mbna_mtodeglon,&mbna_mtodeglat);
			
		/* load sections */
		sprintf(message,"Loading section 1 of crossing %d...",mbna_current_crossing);
		do_message_on(message);
		status = mbnavadjust_section_load(mbna_file_id_1, mbna_section_1, (void **) &swathraw1, (void **) &swath1, section1->num_pings);
		sprintf(message,"Loading section 2 of crossing %d...",mbna_current_crossing);
		do_message_on(message);
		status = mbnavadjust_section_load(mbna_file_id_2, mbna_section_2, (void **) &swathraw2, (void **) &swath2, section2->num_pings);
			
		/* get lon lat positions for soundings */
		sprintf(message,"Transforming section 1 of crossing %d...",mbna_current_crossing);
		do_message_on(message);
		status = mbnavadjust_section_translate(mbna_file_id_1, swathraw1, swath1, 0.0);
		sprintf(message,"Transforming section 2 of crossing %d...",mbna_current_crossing);
		do_message_on(message);
		status = mbnavadjust_section_translate(mbna_file_id_2, swathraw2, swath2, mbna_offset_z);
	
		/* generate contour data */
		sprintf(message,"Contouring section 1 of crossing %d...",mbna_current_crossing);
		do_message_on(message);
		status = mbnavadjust_section_contour(mbna_file_id_1,mbna_section_1,swath1,&mbna_contour1);
		sprintf(message,"Contouring section 2 of crossing %d...",mbna_current_crossing);
		do_message_on(message);
		status = mbnavadjust_section_contour(mbna_file_id_2,mbna_section_2,swath2,&mbna_contour2);
		
		/* generate misfit grids */
		sprintf(message,"Getting misfit for crossing %d...",mbna_current_crossing);
		do_message_on(message);
		status = mbnavadjust_get_misfit();

		/* set loaded flag */
		mbna_naverr_load = MB_YES;
	
		/* turn off message */
		if (mbna_status == MBNA_STATUS_NAVERR)
			do_message_off();
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
	if (mbna_status == MBNA_STATUS_NAVERR
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
		fprintf(stderr,"dbg2       swath_ptr:    %d  %d\n",swath_ptr, *swath_ptr);
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
				    pingraw->draft = draft;
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

/*fprintf(stderr, "%d  %4d/%2d/%2d %2d:%2d:%2d.%6.6d  %11.6f %11.6f %d:%d\n",
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
		fprintf(stderr,"dbg2       swathraw_ptr: %d\n",swathraw_ptr);
		fprintf(stderr,"dbg2       swath_ptr:    %d\n",swath_ptr);
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
	FILE	*cfp;
	char	path[STRING_MAX];
	int	index;
	char	buffer[128];
	int	done;
	int	versionmajor, versionminor;
	double	contour_interval, color_interval, tick_interval, label_interval;
	int	nvector, unused;
	int	i;
	

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       fileid:       %d\n",fileid);
		fprintf(stderr,"dbg2       sectionid:    %d\n",sectionid);
		fprintf(stderr,"dbg2       swath:        %d\n",swath);
		fprintf(stderr,"dbg2       contour:      %d\n",contour);
		fprintf(stderr,"dbg2       nvector:      %d\n",contour->nvector);
		fprintf(stderr,"dbg2       nvector_alloc:%d\n",contour->nvector_alloc);
		}

	if (swath != NULL)
		{
		/* set vectors */
		mbna_contour = contour;
		contour->nvector = 0;
		done = MB_NO;
		
		/* set contour file path */
		sprintf(path,"%s/nvs_%4.4d_%4.4d.cntr",
			project.datadir,fileid,sectionid);
		    
		/* because we are now doing z offsets we always have to recontour */
		project.files[fileid].sections[sectionid].contoursuptodate = MB_NO;
		
		/* read contours from file if possible */
		if (project.files[fileid].sections[sectionid].contoursuptodate == MB_YES 
		    && (cfp = fopen(path,"r")) != NULL)
		    {
		    /* read contour file header */
		    index = 4 * sizeof(int) + 4 * sizeof(double);
		    fread(buffer, 1, index, cfp);
		    index = 0;
		    mb_get_binary_int(MB_NO, &buffer[index], &versionmajor); index += sizeof(int);
		    mb_get_binary_int(MB_NO, &buffer[index], &versionminor); index += sizeof(int);
		    mb_get_binary_double(MB_NO, &buffer[index], &contour_interval); index += sizeof(double);
		    mb_get_binary_double(MB_NO, &buffer[index], &color_interval); index += sizeof(double);
		    mb_get_binary_double(MB_NO, &buffer[index], &tick_interval); index += sizeof(double);
		    mb_get_binary_double(MB_NO, &buffer[index], &label_interval); index += sizeof(double);
		    mb_get_binary_int(MB_NO, &buffer[index], &nvector); index += sizeof(int);
		    mb_get_binary_int(MB_NO, &buffer[index], &unused); index += sizeof(int);
		    
		    /* read contours if parameters are the same as at present */
		    if (contour_interval == project.cont_int
		    	&& color_interval == project.col_int
		    	&& tick_interval == project.tick_int)
			{
			/* make sure space is allocated for contours */
			if (contour->nvector_alloc < nvector)
				{
	    			contour->vector = (struct mbna_plot_vector *)
	    						realloc(contour->vector,
								sizeof(struct mbna_plot_vector) * nvector);
	    			if (contour->vector != NULL)
					contour->nvector_alloc = nvector;
				else
					contour->nvector_alloc = 0;
				}
			
			/* read contour plot vectors */
	    		if (contour->vector != NULL)
				{
				for (i=0;i<nvector;i++)
					{
		    			index = 2 * sizeof(int) + 2 * sizeof(double);
					fread(buffer, 1, index, cfp);
					index = 0;
					mb_get_binary_int(MB_NO, &buffer[index], &contour->vector[i].command); index += sizeof(int);
					mb_get_binary_int(MB_NO, &buffer[index], &contour->vector[i].color); index += sizeof(int);
					mb_get_binary_double(MB_NO, &buffer[index], &contour->vector[i].x); index += sizeof(double);
					mb_get_binary_double(MB_NO, &buffer[index], &contour->vector[i].y); index += sizeof(double);
					contour->nvector++;
					}
				
				/* set done flag so that contours aren't regenerated */
				done = MB_YES;
				}
			}

		    /* close the contour file */
		    fclose(cfp);
		    }
		
		/* plot data if needed */
		if (done == MB_NO)
			{
    			/* reset contouring parameters */
			swath->contour_int = project.cont_int;
			swath->color_int = project.col_int;
			swath->tick_int = project.tick_int;

			/* generate contours */
			status = mb_contour(mbna_verbose,swath,&error);

			/* write out contour file if possible */
			if ((cfp = fopen(path,"w")) != NULL)
			    {
			    /* write contour file header */
			    index = 0;
			    mb_put_binary_int(MB_NO, 1, &buffer[index]); index += sizeof(int);
			    mb_put_binary_int(MB_NO, 0, &buffer[index]); index += sizeof(int);
			    mb_put_binary_double(MB_NO, project.cont_int, &buffer[index]); index += sizeof(double);
			    mb_put_binary_double(MB_NO, project.col_int, &buffer[index]); index += sizeof(double);
			    mb_put_binary_double(MB_NO, project.tick_int, &buffer[index]); index += sizeof(double);
			    mb_put_binary_double(MB_NO, project.label_int, &buffer[index]); index += sizeof(double);
			    mb_put_binary_int(MB_NO, contour->nvector, &buffer[index]); index += sizeof(int);
			    mb_put_binary_int(MB_NO, 0, &buffer[index]); index += sizeof(int);
			    fwrite(buffer, 1, index, cfp);

			    /* write contour plot vectors */
			    for (i=0;i<contour->nvector;i++)
		    		{
		        	index = 0;
		        	mb_put_binary_int(MB_NO, contour->vector[i].command, &buffer[index]); index += sizeof(int);
		        	mb_put_binary_int(MB_NO, contour->vector[i].color, &buffer[index]); index += sizeof(int);
		        	mb_put_binary_double(MB_NO, contour->vector[i].x, &buffer[index]); index += sizeof(double);
		        	mb_put_binary_double(MB_NO, contour->vector[i].y, &buffer[index]); index += sizeof(double);
		        	fwrite(buffer, 1, index, cfp);
				}

			    /* close the file */
			    fclose(cfp);
			    
			    /* set the contours up to date flag */
			    project.files[fileid].sections[sectionid].contoursuptodate = MB_YES;
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
	    		dx = (section->snav_lon[i] + mbna_offset_x - x) / mbna_mtodeglon;
	    		dy = (section->snav_lat[i] + mbna_offset_y - y) / mbna_mtodeglat;
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
	    		dx = (section->snav_lon[i] - x) / mbna_mtodeglon;
	    		dy = (section->snav_lat[i] - y) / mbna_mtodeglat;
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
		fprintf(stderr,"dbg2       mbna_snav_1_lon:    %f\n",mbna_snav_1_lon);
		fprintf(stderr,"dbg2       mbna_snav_1_lat:    %f\n",mbna_snav_1_lat);
		fprintf(stderr,"dbg2       section->num_snav:  %d\n",section->num_snav);
		for (i=0;i<section->num_snav;i++)
			{
			fprintf(stderr,"dbg2       section1->snav_time_d[%d]: %f\n",i,section->snav_time_d[i]);
			fprintf(stderr,"dbg2       section1->snav_lon[%d]:    %f\n",i,section->snav_lon[i]);
			fprintf(stderr,"dbg2       section1->snav_lat[%d]:    %f\n",i,section->snav_lat[i]);
			}
		section = &project.files[crossing->file_id_2].sections[crossing->section_2];
		fprintf(stderr,"dbg2       mbna_snav_2:        %d\n",mbna_snav_2);
		fprintf(stderr,"dbg2       mbna_snav_2_time_d: %f\n",mbna_snav_2_time_d);
		fprintf(stderr,"dbg2       mbna_snav_2_lon:    %f\n",mbna_snav_2_lon);
		fprintf(stderr,"dbg2       mbna_snav_2_lat:    %f\n",mbna_snav_2_lat);
		fprintf(stderr,"dbg2       section->num_snav:  %d\n",section->num_snav);
		for (i=0;i<section->num_snav;i++)
			{
			fprintf(stderr,"dbg2       section2->snav_time_d[%d]: %f\n",i,section->snav_time_d[i]);
			fprintf(stderr,"dbg2       section2->snav_lon[%d]:    %f\n",i,section->snav_lon[i]);
			fprintf(stderr,"dbg2       section2->snav_lat[%d]:    %f\n",i,section->snav_lat[i]);
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
	xa1 = section->snav_lon[0];
	ya1 = section->snav_lat[0];
	xa2 = section->snav_lon[section->num_snav - 1];
	ya2 = section->snav_lat[section->num_snav - 1];
	file = &project.files[crossing->file_id_2];
	section = &file->sections[crossing->section_2];
	xb1 = section->snav_lon[0];
	yb1 = section->snav_lat[0];
	xb2 = section->snav_lon[section->num_snav - 1];
	yb2 = section->snav_lat[section->num_snav - 1];
	
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
int mbnavadjust_crossing_overlap(int crossing_id, 
				double *lonmin, double *lonmax, 
				double *latmin, double *latmax)
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
	int	overlap = 0;
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
		fprintf(stderr,"dbg2       crossing_id:  %d\n",crossing_id);
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
			    if (section1->coverage[kk2] == 1)
				{
				lon2min = section2->lonmin + dx2 * ii2;
				lon2max = section2->lonmin + dx2 * (ii2 + 1);
				lat2min = section2->latmin + dy2 * jj2;
				lat2max = section2->latmin + dy2 * (jj2 + 1);
				if ((lon1min < lon2max)
				    && (lon1max > lon2min)
				    && (lat1min < lat2max)
				    && (lat1max > lat2min))
				    {
				    overlap1[kk1] = 1;
				    overlap2[kk2] = 1;
				    if (lonmin != NULL)
				    	{
					if (first == MB_NO)
				    	    {
					    *lonmin = MIN(*lonmin, MIN(lon1min, lon2min));
					    *lonmax = MAX(*lonmax, MAX(lon1max, lon2max));
					    *latmin = MIN(*latmin, MIN(lat1min, lat2min));
					    *latmax = MAX(*latmax, MAX(lat1max, lat2max));
					    }
					else
					    {
					    first = MB_NO;
					    *lonmin = MIN(lon1min, lon2min);
					    *lonmax = MAX(lon1max, lon2max);
					    *latmin = MIN(lat1min, lat2min);
					    *latmax = MAX(lat1max, lat2max);
					    }
					}
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
		if (lonmin != NULL)
			{
			fprintf(stderr,"dbg2       lonmin:      %f\n",*lonmin);
			fprintf(stderr,"dbg2       lonmax:      %f\n",*lonmax);
			fprintf(stderr,"dbg2       latmin:      %f\n",*latmin);
			fprintf(stderr,"dbg2       latmax:      %f\n",*latmax);
			}
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
	double	x1[3], x2[3], x3[3];
	double	r1, r2, r3;
	double	x, y, z, r;
	double	dotproductsave2;
	double	rsave2;
	double	dotproductsave3;
	double	rsave3;
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
    		&& mbna_current_crossing >= 0)
    		{
    		/* set message on */
    		if (mbna_verbose > 1)
			fprintf(stderr,"Making misfit grid for crossing %d\n",mbna_current_crossing);
		sprintf(message,"Making misfit grid for crossing %d\n",mbna_current_crossing);
		do_message_on(message);
		
		/* figure out lateral extent of grids */
		grid_nx = MBNA_MISFIT_DIMXY;
		grid_ny = MBNA_MISFIT_DIMXY;
		if ((mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon
		    > (mbna_plot_lat_max - mbna_plot_lat_min) / mbna_mtodeglat)
		    {
		    grid_dx = (mbna_plot_lon_max - mbna_plot_lon_min) / (grid_nx - 1);
		    grid_dy = grid_dx * mbna_mtodeglat / mbna_mtodeglon;
		    }
		else
		    {
		    grid_dy = (mbna_plot_lat_max - mbna_plot_lat_min) / (grid_ny - 1);
		    grid_dx = grid_dy * mbna_mtodeglon / mbna_mtodeglat;
		    }
		grid_nxy = grid_nx * grid_ny;
		grid_olon = 0.5 * (mbna_plot_lon_min + mbna_plot_lon_max)
				    - (grid_nx / 2 + 0.5) * grid_dx;
		grid_olat = 0.5 * (mbna_plot_lat_min + mbna_plot_lat_max)
				    - (grid_ny / 2 + 0.5) * grid_dy;
		mbna_misfit_lon_min = grid_olon - grid_dx * grid_nx / 2;
		mbna_misfit_lon_max = grid_olon - grid_dx * grid_nx / 2;
		mbna_misfit_lat_min = grid_olat - grid_dy * grid_ny / 2;
		mbna_misfit_lat_max = grid_olat - grid_dy * grid_ny / 2;

		/* figure out range of z offsets */
		zmin = mbna_misfit_offset_z - 0.5 * project.zoffsetwidth;
		zmax = mbna_misfit_offset_z + 0.5 * project.zoffsetwidth;
		zoff_dz = project.zoffsetwidth / (nzmisfitcalc - 1);
		
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
/*fprintf(stderr, "GRID parameters: dx:%f nx:%d ny:%d  bounds:  grid: %f %f %f %f  plot: %f %f %f %f\n",
grid_dx, grid_nx, grid_ny,
grid_olon, grid_olon + grid_nx * grid_dx,
grid_olat, grid_olat + grid_ny * grid_dy,
mbna_lon_min, mbna_lon_max, mbna_lat_min, mbna_lat_max);*/

		/* figure out range of z offsets */
		zmin = mbna_misfit_offset_z - 0.5 * project.zoffsetwidth;
		zmax = mbna_misfit_offset_z + 0.5 * project.zoffsetwidth;
		zoff_dz = project.zoffsetwidth / (nzmisfitcalc - 1);

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
	    				x = (swath1->pings[i].bathlon[j] + mbna_misfit_offset_x - grid_olon);
	    				y = (swath1->pings[i].bathlat[j] + mbna_misfit_offset_y - grid_olat);
					igx = (int) (x / grid_dx);
					igy = (int) (y / grid_dy);
					k = igx + igy * grid_nx;
					if (igx >= 0 && igx < grid_nx
					    && igy >= 0 && igy < grid_ny)
					    {
					    grid1[k] += swath1->pings[i].bath[j];
					    gridn1[k] ++;
					    }
/*else
fprintf(stderr, "BAD swath1: %d %d  %f %f  %f %f  %d %d\n",
i, j, swath1->pings[i].bathlon[j], swath1->pings[i].bathlat[j], x, y, igx, igy);*/					
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
	    				x = (swath2->pings[i].bathlon[j] - grid_olon);
	    				y = (swath2->pings[i].bathlat[j] - grid_olat);
					igx = (int) (x / grid_dx);
					igy = (int) (y / grid_dy);
					k = igx + igy * grid_nx;
					if (igx >= 0 && igx < grid_nx
					    && igy >= 0 && igy < grid_ny)
					    {
					    grid2[k] += swath2->pings[i].bath[j];
					    gridn2[k] ++;
					    }
/*else
fprintf(stderr, "BAD swath2: %d %d  %f %f  %f %f  %d %d\n",
i, j, swath2->pings[i].bathlon[j], swath2->pings[i].bathlat[j], x, y, igx, igy);*/					
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
		    }		
			
		/* calculate gridded misfit over lateral and z offsets */
		misfit_min = 0.0;
		misfit_max = 0.0;
		mbna_minmisfit = 0.0;
		mbna_minmisfit_n = 0;
		mbna_minmisfit_x = 0.0;
		mbna_minmisfit_y = 0.0;
		mbna_minmisfit_z = 0.0;
		for (ic=0;ic<gridm_nx;ic++)
		    for (jc=0;jc<gridm_ny;jc++)
			for (kc=0;kc<nzmisfitcalc;kc++)
			    {
			    lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
			    gridm[lc] = 0.0;
			    gridnm[lc] = 0;

			    ioff = ic - (gridm_nx / 2);
			    joff = jc - (gridm_ny / 2);
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
					gridm[lc] += (grid1[k1] - grid2[k2] - zoff + mbna_offset_z)
							* (grid1[k1] - grid2[k2] - zoff + mbna_offset_z);
					gridnm[lc]++;
					}
				    }
			    if (gridnm[lc] > 0)
				{
 				gridm[lc] = sqrt(gridm[lc]) / gridnm[lc];
				if (misfit_max == 0.0)
			    	    {
				    misfit_min = gridm[lc];
				    }
				if (gridnm[lc] > mbna_minmisfit_nthreshold
				    && (mbna_minmisfit_n == 0 || gridm[lc] < mbna_minmisfit))
				    {
				    mbna_minmisfit = gridm[lc];
				    mbna_minmisfit_n = gridnm[lc];
				    mbna_minmisfit_x = (ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x;
				    mbna_minmisfit_y = (jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y;
				    mbna_minmisfit_z = zoff;
				    imin = ic;
				    jmin = jc,
				    kmin = kc;
/*fprintf(stderr,"ic:%d jc:%d misfit:%f  pos:%f %f\n",
ic,jc,misfit_min,mbna_minmisfit_x,mbna_minmisfit_y); */
 			    	    }
				misfit_min = MIN(misfit_min, gridm[lc]);
				misfit_max = MAX(misfit_max, gridm[lc]);
				}
			    }
		misfit_min = 0.99 * misfit_min;
		misfit_max = 1.01 * misfit_max;
/*lc = kmin + nzmisfitcalc * (imin + jmin * gridm_nx);
fprintf(stderr, "min misfit: i:%d j:%d k:%d    n:%d m:%f   offsets: %f %f %f\n",
imin, jmin, kmin, gridnm[lc], gridm[lc],
mbna_minmisfit_x / mbna_mtodeglon, 
mbna_minmisfit_y / mbna_mtodeglat, 
mbna_minmisfit_z);*/

/*fprintf(stderr,"Misfit bounds: nmin:%d best:%f min:%f max:%f min loc: %f %f %f\n",
mbna_minmisfit_n,mbna_minmisfit,misfit_min,misfit_max,
mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z);*/
		
		/* get minimum misfit in 2D plane at current z offset */
		mbnavadjust_get_misfitxy();

    		/* set message on */
    		if (mbna_verbose > 1)
			fprintf(stderr,"Histogram equalizing misfit grid for crossing %d\n",mbna_current_crossing);
		sprintf(message,"Histogram equalizing misfit grid for crossing %d\n",mbna_current_crossing);
		do_message_on(message);
		
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
		qsort((char *)gridmeq,grid_nxyzeq,sizeof(double),
				(void *)mb_double_compare);
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
		
   		/* set message on */
    		if (mbna_verbose > 1)
			fprintf(stderr,"Estimating 3D uncertainty for crossing %d\n",mbna_current_crossing);
		sprintf(message,"Estimating 3D uncertainty for crossing %d\n",mbna_current_crossing);
		do_message_on(message);
		
		/* estimating 3 component uncertainty vector at minimum misfit point */
		if (grid_nxyzeq > 0)
		    {
		
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
/*fprintf(stderr,"%d %d %d gridm[%d]:%f minmisfitthreshold:%f x: %f %f %f  r:%f\n",
ic,jc,kc,lc,gridm[lc],minmisfitthreshold,x,y,z,r);*/
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
/*fprintf(stderr,"Longest vector in misfit space. %f %f %f  r:%f\n",mbna_minmisfit_sx1[0],mbna_minmisfit_sx1[1],mbna_minmisfit_sx1[2],mbna_minmisfit_sr1);*/

		    /* now get a horizontal unit vector perpendicular to the the longest vector 
			    and then find the largest r associated with that vector */
		    mbna_minmisfit_sr2 = sqrt(mbna_minmisfit_sx1[0] * mbna_minmisfit_sx1[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx1[1]);
		    if (mbna_minmisfit_sr2 == 0.0)
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
/*dotproduct = (mbna_minmisfit_sx1[0] * mbna_minmisfit_sx2[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx2[1] + mbna_minmisfit_sx1[2] * mbna_minmisfit_sx2[2]);
fprintf(stderr,"Horizontal perpendicular vector in misfit space. %f %f %f  r:%f dotproduct:%f\n",
mbna_minmisfit_sx2[0],mbna_minmisfit_sx2[1],mbna_minmisfit_sx2[2],mbna_minmisfit_sr2,dotproduct);*/

		    /* now get a near-vertical unit vector perpendicular to the the longest vector 
			    and then find the largest r associated with that vector */
		    mbna_minmisfit_sr3 = sqrt(mbna_minmisfit_sx1[0] * mbna_minmisfit_sx1[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx1[1]);
		    if (mbna_minmisfit_sr3 == 0.0)
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
/*dotproduct = (mbna_minmisfit_sx1[0] * mbna_minmisfit_sx3[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx3[1] + mbna_minmisfit_sx1[2] * mbna_minmisfit_sx3[2]);
fprintf(stderr,"Perpendicular near-vertical vector in misfit space. %f %f %f  r:%f dotproduct:%f\n",
mbna_minmisfit_sx3[0],mbna_minmisfit_sx3[1],mbna_minmisfit_sx3[2],mbna_minmisfit_sr2,dotproduct);*/

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
/*fprintf(stderr,"Vector2: %d %d %d gridm[%d]:%f minmisfitthreshold:%f dotproduct:%f x: %f %f %f  r:%f\n",
ic,jc,kc,lc,gridm[lc],minmisfitthreshold,dotproduct,x,y,z,r);*/
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
/*fprintf(stderr,"Vector3: %d %d %d gridm[%d]:%f minmisfitthreshold:%f dotproduct:%f x: %f %f %f  r:%f\n",
ic,jc,kc,lc,gridm[lc],minmisfitthreshold,dotproduct,x,y,z,r);*/
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
		    if (mbna_minmisfit_sr2 <= 0.0)
		    	mbna_minmisfit_sr2 = rsave2;
		    if (mbna_minmisfit_sr3 <= 0.0)
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
/*fprintf(stderr,"\nVector1: %f %f %f  mbna_minmisfit_sr1:%f\n",
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
fprintf(stderr,"3v2:%f\n",dotproduct);*/
		
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
    		&& mbna_current_crossing >= 0)
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
		    }
		}
fprintf(stderr,"mbna_minmisfit_xh:%f mbna_minmisfit_yh:%f mbna_minmisfit_zh:%f\n",
mbna_minmisfit_xh,mbna_minmisfit_yh,mbna_minmisfit_zh);
			
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
void plot(double xx,double yy,int ipen)
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
void newpen(int icolor)
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
void justify_string(double height,char *string, double *s)
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
void plot_string(double x, double y, double hgt, double angle, char *label)
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
	    xscale = (corr_borders[1] - corr_borders[0])
		    / (grid_dx * (gridm_nx - 1));
	    yscale = (corr_borders[3] - corr_borders[2])
		    / (grid_dy * (gridm_ny - 1));
	    mbna_misfit_scale = MIN(xscale, yscale);
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
	double	xscale, yscale;
	int 	ix, iy, idx, idy;
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
			    pixel_values[WHITE], XG_SOLIDLINE);
		xg_fillrectangle(pcorr_xgid, 0, 0,
			    corr_borders[1], corr_borders[3],
			    pixel_values[WHITE], XG_SOLIDLINE);
		}
	    xg_fillrectangle(pzoff_xgid, 0, 0,
			    zoff_borders[1], zoff_borders[3],
			    pixel_values[WHITE], XG_SOLIDLINE);
			
	    /* replot section 1 and tie points in white if moving that section */
	    if (plotmode == MBNA_PLOT_MODE_MOVE)
	    {
	    for (i=0;i<mbna_contour1.nvector;i++)
		{
		v = &mbna_contour1.vector[i];
	
		if (v->command == MBNA_PEN_UP)
		    {
		    ixo = (int)(mbna_plotx_scale * (v->x + mbna_offset_x_old - mbna_plot_lon_min));
		    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (v->y + mbna_offset_y_old - mbna_plot_lat_min));
		    }
		else if (v->command == MBNA_PEN_DOWN)
		    {
		    ix = (int)(mbna_plotx_scale * (v->x + mbna_offset_x_old - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (v->y + mbna_offset_y_old - mbna_plot_lat_min));
		    xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[WHITE], XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    }	
		}
	    ixo = (int)(mbna_plotx_scale * (swathraw1->pingraws[0].navlon + mbna_offset_x_old - mbna_plot_lon_min));
	    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw1->pingraws[0].navlat + mbna_offset_y_old - mbna_plot_lat_min));
	    for (i=1;i<swathraw1->npings;i++)
		{
		ix = (int)(mbna_plotx_scale * (swathraw1->pingraws[i].navlon + mbna_offset_x_old - mbna_plot_lon_min));
		iy = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw1->pingraws[i].navlat + mbna_offset_y_old - mbna_plot_lat_min));
		xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[WHITE], XG_SOLIDLINE);
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
		    ix = (int)(mbna_plotx_scale * (section1->snav_lon[snav_1] + mbna_offset_x_old - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (section1->snav_lat[snav_1] + mbna_offset_y_old - mbna_plot_lat_min));
		    xg_fillrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[WHITE], XG_SOLIDLINE);
		    xg_drawrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[WHITE], XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    ix = (int)(mbna_plotx_scale * (section2->snav_lon[snav_2] - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (section2->snav_lat[snav_2] - mbna_plot_lat_min));
		    xg_fillrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[WHITE], XG_SOLIDLINE);
		    xg_drawrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[WHITE], XG_SOLIDLINE);
		    xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[WHITE], XG_SOLIDLINE);
		    }
		}
	    }
	
	    /* replot zoom box in white if moving that box */
	    if (plotmode == MBNA_PLOT_MODE_ZOOM)
		{
		xg_drawrectangle(pcont_xgid,
				    MIN(izx1, izx2),
				    MIN(izy1, izy2),
				    MAX(izx1, izx2) - MIN(izx1, izx2),
				    MAX(izy1, izy2) - MIN(izy1, izy2),
				    pixel_values[WHITE], XG_SOLIDLINE);
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
	    ixo = (int)(mbna_plotx_scale * (swathraw1->pingraws[0].navlon + mbna_offset_x - mbna_plot_lon_min));
	    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw1->pingraws[0].navlat + mbna_offset_y - mbna_plot_lat_min));
	    for (i=1;i<swathraw1->npings;i++)
		{
		ix = (int)(mbna_plotx_scale * (swathraw1->pingraws[i].navlon + mbna_offset_x - mbna_plot_lon_min));
		iy = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw1->pingraws[i].navlat + mbna_offset_y - mbna_plot_lat_min));
		xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[BLACK], XG_SOLIDLINE);
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
	    ixo = (int)(mbna_plotx_scale * (swathraw2->pingraws[0].navlon - mbna_plot_lon_min));
	    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw2->pingraws[0].navlat - mbna_plot_lat_min));
	    for (i=1;i<swathraw2->npings;i++)
		{
		ix = (int)(mbna_plotx_scale * (swathraw2->pingraws[i].navlon - mbna_plot_lon_min));
		iy = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw2->pingraws[i].navlat - mbna_plot_lat_min));
		xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[BLACK], XG_SOLIDLINE);
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
				fill = pixel_values[2];
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
		    ix = (int)(mbna_plotx_scale * (section1->snav_lon[snav_1] + mbna_offset_x - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (section1->snav_lat[snav_1] + mbna_offset_y - mbna_plot_lat_min));
		    xg_fillrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, fill, XG_SOLIDLINE);
		    xg_drawrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[BLACK], XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    ix = (int)(mbna_plotx_scale * (section2->snav_lon[snav_2] - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (section2->snav_lat[snav_2] - mbna_plot_lat_min));
		    xg_fillrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, fill, XG_SOLIDLINE);
		    xg_drawrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[BLACK], XG_SOLIDLINE);
		    xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[BLACK], XG_SOLIDLINE);
		    }
		}
	
	    /* plot zoom box if in zoom mode */
	    if (plotmode == MBNA_PLOT_MODE_ZOOMFIRST || plotmode == MBNA_PLOT_MODE_ZOOM)
		{
		xg_drawrectangle(pcont_xgid,
				    MIN(mbna_zoom_x1, mbna_zoom_x2),
				    MIN(mbna_zoom_y1, mbna_zoom_y2),
				    MAX(mbna_zoom_x1, mbna_zoom_x2) - MIN(mbna_zoom_x1, mbna_zoom_x2),
				    MAX(mbna_zoom_y1, mbna_zoom_y2) - MIN(mbna_zoom_y1, mbna_zoom_y2),
				    pixel_values[BLACK], XG_SOLIDLINE);
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
			ix = ixo + (int)(mbna_misfit_scale * grid_dx
					    * (i - gridm_nx / 2 - 0.5));
			iy = iyo - (int)(mbna_misfit_scale * grid_dy
					    * (j - gridm_ny / 2 + 0.5));
			idx = ixo + (int)(mbna_misfit_scale * grid_dx
					    * (i - gridm_nx / 2 + 0.5))
				    - ix;
			idy = iyo - (int)(mbna_misfit_scale * grid_dy
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
			    ixo - (int)(mbna_misfit_scale * mbna_misfit_offset_x), 
			    corr_borders[2],
			    ixo - (int)(mbna_misfit_scale * mbna_misfit_offset_x), 
			    corr_borders[3],
			    pixel_values[BLACK], XG_DASHLINE);
	    xg_drawline(pcorr_xgid,
			    corr_borders[0], 
			    iyo + (int)(mbna_misfit_scale * mbna_misfit_offset_y),
			    corr_borders[1], 
			    iyo + (int)(mbna_misfit_scale * mbna_misfit_offset_y),
			    pixel_values[BLACK], XG_DASHLINE);
	
	    /* draw working offset */
	    ix = ixo + (int)(mbna_misfit_scale * (mbna_offset_x - mbna_misfit_offset_x));
	    iy = iyo - (int)(mbna_misfit_scale * (mbna_offset_y - mbna_misfit_offset_y));
	    xg_fillrectangle(pcorr_xgid, ix-3, iy-3, 7, 7, pixel_values[2], XG_SOLIDLINE);
	    xg_drawrectangle(pcorr_xgid, ix-3, iy-3, 7, 7, pixel_values[BLACK], XG_SOLIDLINE);
	    
	    /* draw uncertainty estimate */
	    if (mbna_minmisfit_n > 0)
	    	{
		ix = ixo + (int)(mbna_misfit_scale * (mbna_minmisfit_x - mbna_misfit_offset_x));
		iy = iyo - (int)(mbna_misfit_scale * (mbna_minmisfit_y - mbna_misfit_offset_y));
		idx = (int)(mbna_misfit_scale * (mbna_mtodeglon * mbna_minmisfit_sr1 * mbna_minmisfit_sx1[0]));
		idy = -(int)(mbna_misfit_scale * (mbna_mtodeglat * mbna_minmisfit_sr1 * mbna_minmisfit_sx1[1]));
		xg_drawline(pcorr_xgid,
				ix - idx, iy - idy,
				ix + idx, iy + idy,
				pixel_values[WHITE], XG_SOLIDLINE);

		ix = ixo + (int)(mbna_misfit_scale * (mbna_minmisfit_x - mbna_misfit_offset_x));
		iy = iyo - (int)(mbna_misfit_scale * (mbna_minmisfit_y - mbna_misfit_offset_y));
		idx = (int)(mbna_misfit_scale * (mbna_mtodeglon * mbna_minmisfit_sr2 * mbna_minmisfit_sx2[0]));
		idy = -(int)(mbna_misfit_scale * (mbna_mtodeglat * mbna_minmisfit_sr2 * mbna_minmisfit_sx2[1]));
		xg_drawline(pcorr_xgid,
				ix - idx, iy - idy,
				ix + idx, iy + idy,
				pixel_values[WHITE], XG_SOLIDLINE);
		}	    
	
	    /* draw x at minimum misfit */
	    if (mbna_minmisfit_n > 0)
	    	{
		ix = ixo + (int)(mbna_misfit_scale * (mbna_minmisfit_x - mbna_misfit_offset_x));
		iy = iyo - (int)(mbna_misfit_scale * (mbna_minmisfit_y - mbna_misfit_offset_y));
		xg_drawline(pcorr_xgid,
				ix - 10, iy + 10,
				ix + 10, iy - 10,
				pixel_values[BLACK], XG_SOLIDLINE);
		xg_drawline(pcorr_xgid,
				ix + 10, iy + 10,
				ix - 10, iy - 10,
				pixel_values[BLACK], XG_SOLIDLINE);
		}
	
	    /* draw small x at minimum misfit for current z offset */
	    if (mbna_minmisfit_n > 0)
	    	{
		ix = ixo + (int)(mbna_misfit_scale * (mbna_minmisfit_xh - mbna_misfit_offset_x));
		iy = iyo - (int)(mbna_misfit_scale * (mbna_minmisfit_yh - mbna_misfit_offset_y));
		xg_drawline(pcorr_xgid,
				ix - 5, iy + 5,
				ix + 5, iy - 5,
				pixel_values[BLACK], XG_SOLIDLINE);
		xg_drawline(pcorr_xgid,
				ix + 5, iy + 5,
				ix - 5, iy - 5,
				pixel_values[BLACK], XG_SOLIDLINE);
		}
	
	    /* draw + at inversion solution */
	    if (project.inversion != MBNA_INVERSION_NONE)
	    	{
	    	ix = ixo + (int)(mbna_misfit_scale * (mbna_invert_offset_x - mbna_misfit_offset_x));
	    	iy = iyo - (int)(mbna_misfit_scale * (mbna_invert_offset_y - mbna_misfit_offset_y));
	    	xg_drawline(pcorr_xgid,
			    ix - 10, iy,
			    ix + 10, iy,
			    pixel_values[3], XG_SOLIDLINE);
	    	xg_drawline(pcorr_xgid,
			    ix, iy + 10,
			    ix, iy - 10,
			    pixel_values[3], XG_SOLIDLINE);
	    	xg_drawline(pcorr_xgid,
			    ix - 10, iy,
			    ix + 10, iy,
			    pixel_values[BLACK], XG_SOLIDLINE);
	    	xg_drawline(pcorr_xgid,
			    ix, iy + 10,
			    ix, iy - 10,
			    pixel_values[BLACK], XG_SOLIDLINE);
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
			    pixel_values[BLACK], XG_DASHLINE);

	    /* draw working offset */
	    ix = ixo + (int)(mbna_zoff_scale_x * (mbna_offset_z - zmin));
	    xg_drawline(pzoff_xgid,
			    ix, zoff_borders[2],
			    ix, zoff_borders[3],
			    pixel_values[BLACK], XG_SOLIDLINE);
	
	    /* draw x at minimum misfit */
	    if (mbna_minmisfit_n > 0)
	    	{
		ix = ixo + (int)(mbna_zoff_scale_x * (mbna_minmisfit_z - zmin));
		iy = zoff_borders[3] / 2;
		xg_drawline(pzoff_xgid,
				ix - 10, iy + 10,
				ix + 10, iy - 10,
				pixel_values[BLACK], XG_SOLIDLINE);
		xg_drawline(pzoff_xgid,
				ix + 10, iy + 10,
				ix - 10, iy - 10,
				pixel_values[BLACK], XG_SOLIDLINE);
		}
	
	    /* draw + at inversion solution */
	    if (project.inversion != MBNA_INVERSION_NONE)
	    	{
	    	ix = ixo + (int)(mbna_zoff_scale_x * (mbna_invert_offset_z - zmin));
	    	iy = zoff_borders[3] / 2;
	    	xg_drawline(pzoff_xgid,
			    ix - 10, iy,
			    ix + 10, iy,
			    pixel_values[BLACK], XG_SOLIDLINE);
	    	xg_drawline(pzoff_xgid,
			    ix, iy + 10,
			    ix, iy - 10,
			    pixel_values[BLACK], XG_SOLIDLINE);
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
mbnavadjust_autopick()
{
	/* local variables */
	char	*function_name = "mbnavadjust_autopick";
	int	status = MB_SUCCESS;
	struct 	mbna_file *file;
	struct 	mbna_crossing *crossing;
	struct 	mbna_tie *tie;
	double	firstsonardepth1, firsttime_d1, secondsonardepth1, secondtime_d1, dsonardepth1;
	double	firstsonardepth2, firsttime_d2, secondsonardepth2, secondtime_d2, dsonardepth2;
	int	found, process;
	int	i, j, k;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* make sure that all sections referenced in crossings have up-to-date contours made */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
		/* set message dialog on */
		sprintf(message,"Autopicking offsets...");
		do_message_on(message);
		sprintf(message,"Autopicking offsets.\n");
		do_info_add(message,MB_YES);
		
		/* loop over all crossings */
		for (i=0;i<project.num_crossings;i++)
			{
			/* get structure */
			crossing = &(project.crossings[i]);
			
			/* check if processing should proceed */
			process = MB_NO;
			if (crossing->status == MBNA_CROSSING_STATUS_NONE)
				{
				if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS)
					{
					if (crossing->truecrossing == MB_YES)
						process = MB_YES;
					}
				else if (crossing->overlap >= MBNA_OVERLAP_THRESHOLD 
					|| crossing->truecrossing == MB_YES)
					process = MB_YES;
				}
			
			/* load the crossing */
			if (process == MB_YES)
				{
				mbna_current_crossing = i;
    				mbna_file_id_1 = crossing->file_id_1;
    				mbna_section_1 = crossing->section_1;
     				mbna_file_id_2 = crossing->file_id_2;
    				mbna_section_2 = crossing->section_2;
				mbna_current_tie = -1;
				
				/* update status */
				do_update_status();

				/* set message dialog on */
				sprintf(message,"Loading crossing %d...", mbna_current_crossing);
				fprintf(stderr,"%s: %s\n",function_name,message);
				do_message_on(message);

    				/* load crossing */
  				mbnavadjust_crossing_load();
				
				/* get overlap region */
				mbnavadjust_crossing_overlap(i, &mbna_plot_lon_min, &mbna_plot_lon_max, 
								&mbna_plot_lat_min, &mbna_plot_lat_max);
	    
				/* get naverr plot scaling */
				mbnavadjust_naverr_scale();
	    	
				/* get misfit */
				mbnavadjust_get_misfit();
				
				/* set offsets to minimum misfit */
				mbna_offset_x = mbna_minmisfit_x;
				mbna_offset_y = mbna_minmisfit_y;
				mbna_offset_z = mbna_minmisfit_z;
				
				/* add tie */
    				mbnavadjust_naverr_addtie();
				
				/* update status */
				do_update_status();
			
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
fprintf(stderr,"crossing:%d tie:%d zoffset:%f   sdrate1:%f %f %f  sdrate2:%f %f %f   inferred time lag:%f\n",
i,j,tie->offset_z_m, 
(secondsonardepth1 - firstsonardepth1), (secondtime_d1 - firsttime_d1), dsonardepth1,
(secondsonardepth2 - firstsonardepth2), (secondtime_d2 - firsttime_d2), dsonardepth2,
tie->offset_z_m / (dsonardepth2 - dsonardepth1));
					}

    				/* unload crossing */
  				mbnavadjust_crossing_unload();
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
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	nnav;
	int	nsnav;
	int	ndf = 3;
	int	nblock = 0;
	int	ncols = 0;
	int	ntie = 0;
	double	*x = NULL;
	double	*xx = NULL;
	double	misfit_initial;
	double	misfit, misfit_ties;
	double	offsetx, offsety, offsetz, offsetr;
	double	offsetsigma;
	double	weight;
	double	dtime_d, time_d_old, time_d_older;
	int	done, iter;
	int	nseq;
	int	ndx, ndx2;
	int	icrossing, ifile;
	int	isection, isnav;
	int	nc1, nc2, nc3;
	int	i, j, k;

	double	perturbationsize;
	double	smoothweight_best;
	double	sigma_crossing_best;
	double	sigma_total_best;
	double	offset_x;
	double	offset_y;
	double	offset_z;
	int	nc;
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
	double	time_d1, time_d2, time_d3;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* invert if there is a project and all crossings have been analyzed */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed == project.num_crossings
			|| project.num_truecrossings_analyzed == project.num_truecrossings))
			
    		{
		/* set message dialog on */
		sprintf(message,"Setting up navigation inversion...");
		do_message_on(message);
		/*----------------------------------------------------------------*/
		
		/* figure out the average offsets between connected sets of files 
			- invert for x y z offsets for the blocks */
			
		/* count the number of blocks */
		nblock = 0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    if (i==0 || file->sections[0].continuity == MB_NO)
		    	{
			nblock++;
			}
		    file->block = nblock - 1;
		    file->block_offset_x = 0.0;
		    file->block_offset_y = 0.0;
		    file->block_offset_z = 0.0;
		    }
		ntie = 0;
		misfit_initial = 0.0;
		for (icrossing=0;icrossing<project.num_crossings;icrossing++)
		    {
		    crossing = &project.crossings[icrossing];
		    if (crossing->status == MBNA_CROSSING_STATUS_SET)
		    	{
			ntie += crossing->num_ties;
			for (j=0;j<crossing->num_ties;j++)
				{
				tie = (struct mbna_tie *) &crossing->ties[j];
				misfit_initial += tie->offset_x_m * tie->offset_x_m;
				misfit_initial += tie->offset_y_m * tie->offset_y_m;
				misfit_initial += tie->offset_z_m * tie->offset_z_m;
				}
			}
		    }
		misfit_initial = sqrt(misfit_initial) / ntie;
		
		/* invert for block offsets only if there is more than one block */
		if (nblock > 1)
		    {
		    /* allocate space for the inverse problem */
		    ncols = ndf * nblock;
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&x,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&xx,&error);

		    /* initialize x array */
		    for (i=0;i<ndf*nblock;i++)
			{
			x[i] = 0.0;
			}

		    /* construct the inverse problem */
		    sprintf(message,"Solving for block offsets...");
		    do_message_on(message);
		    
		    done = MB_NO;
		    iter = 0;
		    while (done == MB_NO)
		    	{
			/* initialize xx array */
			for (i=0;i<ndf*nblock;i++)
			    {
			    xx[i] = 0.0;
			    }
		    
			/* loop over crossings getting set ties */
			nc = ndf * nblock;
			ntie = 0;
			for (icrossing=0;icrossing<project.num_crossings;icrossing++)
			    {
			    crossing = &project.crossings[icrossing];

			    /* use only set crossings */
			    if (crossing->status == MBNA_CROSSING_STATUS_SET)
			    for (j=0;j<crossing->num_ties;j++)
				{
				/* get tie */
				tie = (struct mbna_tie *) &crossing->ties[j];
				ntie++;

				/* get block id for first snav point */
				file1 = &project.files[crossing->file_id_1];
				nc1 = file1->block;

				/* get block id for second snav point */
				file2 = &project.files[crossing->file_id_2];
				nc2 = file2->block;

				/* get current offset vector including reduction of block solution */
				offsetx = tie->offset_x_m - (x[3*nc2] + xx[3*nc2] - x[3*nc1] - xx[3*nc1]);
				offsety = tie->offset_y_m - (x[3*nc2+1] + xx[3*nc2+1] - x[3*nc1+1] - xx[3*nc1+1]);
				offsetz = tie->offset_z_m  - (x[3*nc2+2] + xx[3*nc2+2] - x[3*nc1+2] - xx[3*nc1+2]);

				if (file1->status == MBNA_FILE_FIXED)
			    	    {
				    xx[3*nc2]   +=  offsetx;
				    xx[3*nc2+1] +=  offsety;
				    xx[3*nc2+2] +=  offsetz;
				    }
				else if (file2->status == MBNA_FILE_FIXED)
			    	    {
				    xx[3*nc1]   += -offsetx;
				    xx[3*nc1+1] += -offsety;
				    xx[3*nc1+2] += -offsetz;
				    }
				else
			    	    {
				    xx[3*nc1]   += -0.5 * offsetx;
				    xx[3*nc1+1] += -0.5 * offsety;
				    xx[3*nc1+2] += -0.5 * offsetz;
				    xx[3*nc2]   +=  0.5 * offsetx;
				    xx[3*nc2+1] +=  0.5 * offsety;
				    xx[3*nc2+2] +=  0.5 * offsetz;
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
			 iter++;
			 if (perturbationsize / misfit_initial < 0.00001 || iter > 100)
			 	done = MB_YES;
			 }

		    /* output solution */
		    if (mbna_verbose > 1)
		    for (i=0;i<ncols/3;i++)
			{
			fprintf(stderr, "block:%d  offsets: %f %f %f\n",
				i, x[3*i], x[3*i+1], x[3*i+2]);
			}

		    /* extract results */
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			file->block_offset_x = x[3 * file->block];
			file->block_offset_y = x[3 * file->block + 1];
			file->block_offset_z = x[3 * file->block + 2];
/* fprintf(stderr,"file:%d block: %d block offsets: %f %f %f\n",
i,file->block,file->block_offset_x,file->block_offset_y,file->block_offset_z);*/
			}

		    /* deallocate arrays */
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&xx,&error);
		    }
		
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
			    if (section->snav_num_ties[isnav] > 0)
				{
				section->snav_invert_id[isnav] = nnav;
				nnav++;
				}
			    }
			}
		    }
		    
		/* allocate solution guess vector x */
		ncols = ndf * nnav;
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&x,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&xx,&error);
		for (i=0;i<ncols;i++)
			{
			x[i] = 0.0;
			xx[i] = 0.0;
			}
			
		/* calculate initial weighted misfit */
		misfit_initial = 0.0;
		ntie = 0;
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
			section = &file1->sections[crossing->section_1];
			nc1 = section->snav_invert_id[tie->snav_1];

			/* get absolute id for second snav point */
			file2 = &project.files[crossing->file_id_2];
			section = &file2->sections[crossing->section_2];
			nc2 = section->snav_invert_id[tie->snav_2];
if (nc1 > nsnav - 1 || nc2 > nsnav -1
|| nc1 < 0 || nc2 < 0)
fprintf(stderr, "BAD snav ID: %d %d %d\n", nc1, nc2, nsnav);

			/* get observed offset vector including reduction of block solution */
			offsetx = tie->offset_x_m - file2->block_offset_x + file1->block_offset_x;
			offsety = tie->offset_y_m - file2->block_offset_y + file1->block_offset_y;
			offsetz = tie->offset_z_m - file2->block_offset_z + file1->block_offset_z;

			/* get long axis misfit */
			misfit = mbna_offsetweight / tie->sigmar1
						* (offsetx * tie->sigmax1[0]
							+ offsety * tie->sigmax1[1]
							+ offsetz * tie->sigmax1[2]);
			misfit_initial += misfit * misfit;
/*fprintf(stderr,"Initial Misfit: %d %f %f\n", ntie, misfit, misfit_initial);*/

			/* get horizontal axis misfit */
			misfit = mbna_offsetweight / tie->sigmar2
						* (offsetx * tie->sigmax2[0]
							+ offsety * tie->sigmax2[1]
							+ offsetz * tie->sigmax2[2]);
			misfit_initial += misfit * misfit;
/*fprintf(stderr,"Initial Misfit: %d %f %f\n", ntie, misfit, misfit_initial);*/

			/* get semi-vertical axis misfit */
			misfit = mbna_offsetweight / tie->sigmar3
						* (offsetx * tie->sigmax3[0]
							+ offsety * tie->sigmax3[1]
							+ offsetz * tie->sigmax3[2]);
			misfit_initial += misfit * misfit;
/*fprintf(stderr,"Initial Misfit: %d %f %f\n\n", ntie, misfit, misfit_initial);*/

			ntie++;
			}
		    }
		misfit_initial = sqrt(misfit_initial) / ntie;
/*fprintf(stderr,"ntie:%d misfit_initial:%f\n",ntie,misfit_initial);*/

		/* loop until convergence */
		done = MB_NO;
		iter = 0;
		while (done == MB_NO)
		    {
		    /* set message dialog on */
		    if (iter % 100 == 0)
		    	{
		    	sprintf(message,"Performing inversion iteration %d of %d...", iter, ncols);
		    	do_message_on(message);
		    	}
		
		    for (i=0;i<ncols;i++)
			{
			xx[i] =0.0;
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
			    section = &file1->sections[crossing->section_1];
			    nc1 = section->snav_invert_id[tie->snav_1];

			    /* get absolute id for second snav point */
			    file2 = &project.files[crossing->file_id_2];
			    section = &file2->sections[crossing->section_2];
			    nc2 = section->snav_invert_id[tie->snav_2];

			    /* get current offset vector including reduction of block solution */
			    offsetx = tie->offset_x_m - file2->block_offset_x + file1->block_offset_x
			    		- (x[3*nc2] + xx[3*nc2] - x[3*nc1] - xx[3*nc1]);
			    offsety = tie->offset_y_m - file2->block_offset_y + file1->block_offset_y
			    		- (x[3*nc2+1] + xx[3*nc2+1] - x[3*nc1+1] - xx[3*nc1+1]);
			    offsetz = tie->offset_z_m - file2->block_offset_z + file1->block_offset_z
			    		- (x[3*nc2+2] + xx[3*nc2+2] - x[3*nc1+2] - xx[3*nc1+2]);
			    offsetr = sqrt(offsetx * offsetx + offsety * offsety + offsetz * offsetz);
					
			    /* deal with each component of the error ellipse
			    	- project offset vector onto each component by dot-product
				- weight inversely by size of error for that component */

			    /* deal with long axis */
			    offsetsigma = offsetx * tie->sigmax1[0] 
			    		+ offsety * tie->sigmax1[1]
					+ offsetz * tie->sigmax1[2];
			    if (fabs(offsetsigma) > 0.0)
			    	weight = MAX(mbna_offsetweight / tie->sigmar1, 1.0);
			    else
			    	weight = 1.0;
			    if (file1->status == MBNA_FILE_FIXED)
			    	{
				xx[3*nc2]   +=  weight * offsetsigma * tie->sigmax1[0];
				xx[3*nc2+1] +=  weight * offsetsigma * tie->sigmax1[1];
				xx[3*nc2+2] +=  weight * offsetsigma * tie->sigmax1[2];
				}
			    else if (file2->status == MBNA_FILE_FIXED)
			    	{
				xx[3*nc1]   += -weight * offsetsigma * tie->sigmax1[0];
				xx[3*nc1+1] += -weight * offsetsigma * tie->sigmax1[1];
				xx[3*nc1+2] += -weight * offsetsigma * tie->sigmax1[2];
				}
			    else
			    	{
				xx[3*nc1]   += -0.5 * weight * offsetsigma * tie->sigmax1[0];
				xx[3*nc1+1] += -0.5 * weight * offsetsigma * tie->sigmax1[1];
				xx[3*nc1+2] += -0.5 * weight * offsetsigma * tie->sigmax1[2];
				xx[3*nc2]   +=  0.5 * weight * offsetsigma * tie->sigmax1[0];
				xx[3*nc2+1] +=  0.5 * weight * offsetsigma * tie->sigmax1[1];
				xx[3*nc2+2] +=  0.5 * weight * offsetsigma * tie->sigmax1[2];
				}
/*fprintf(stderr,"long axis:  nc1:%d xx:%f %f %f  nc2:%d xx:%f %f %f\n",
nc1,xx[3*nc1],xx[3*nc1+1],xx[3*nc1+2],
nc2,xx[3*nc2],xx[3*nc2+1],xx[3*nc2+2]);*/

			    /* deal with horizontal axis */
			    offsetsigma = offsetx * tie->sigmax2[0] 
			    		+ offsety * tie->sigmax2[1]
					+ offsetz * tie->sigmax2[2];
			    if (offsetsigma > 0.0)
			    	weight = MAX(mbna_offsetweight / tie->sigmar2, 1.0);
			    else
			    	weight = 1.0;
			    if (file1->status == MBNA_FILE_FIXED)
			    	{
				xx[3*nc2]   +=  weight * offsetsigma * tie->sigmax2[0];
				xx[3*nc2+1] +=  weight * offsetsigma * tie->sigmax2[1];
				xx[3*nc2+2] +=  weight * offsetsigma * tie->sigmax2[2];
				}
			    else if (file2->status == MBNA_FILE_FIXED)
			    	{
				xx[3*nc1]   += -weight * offsetsigma * tie->sigmax2[0];
				xx[3*nc1+1] += -weight * offsetsigma * tie->sigmax2[1];
				xx[3*nc1+2] += -weight * offsetsigma * tie->sigmax2[2];
				}
			    else
			    	{
				xx[3*nc1]   += -0.5 * weight * offsetsigma * tie->sigmax2[0];
				xx[3*nc1+1] += -0.5 * weight * offsetsigma * tie->sigmax2[1];
				xx[3*nc1+2] += -0.5 * weight * offsetsigma * tie->sigmax2[2];
				xx[3*nc2]   +=  0.5 * weight * offsetsigma * tie->sigmax2[0];
				xx[3*nc2+1] +=  0.5 * weight * offsetsigma * tie->sigmax2[1];
				xx[3*nc2+2] +=  0.5 * weight * offsetsigma * tie->sigmax2[2];
				}
/*fprintf(stderr,"horizontal:  nc1:%d xx:%f %f %f  nc2:%d xx:%f %f %f\n",
nc1,xx[3*nc1],xx[3*nc1+1],xx[3*nc1+2],
nc2,xx[3*nc2],xx[3*nc2+1],xx[3*nc2+2]);*/

			    /* deal with semi-vertical axis */
			    offsetsigma = offsetx * tie->sigmax3[0] 
			    		+ offsety * tie->sigmax3[1]
					+ offsetz * tie->sigmax3[2];
			    if (offsetsigma > 0.0)
			    	weight = MAX(mbna_offsetweight / tie->sigmar3, 1.0);
			    else
			    	weight = 1.0;
			    if (file1->status == MBNA_FILE_FIXED)
			    	{
				xx[3*nc2]   +=  weight * offsetsigma * tie->sigmax3[0];
				xx[3*nc2+1] +=  weight * offsetsigma * tie->sigmax3[1];
				xx[3*nc2+2] +=  weight * offsetsigma * tie->sigmax3[2];
				}
			    else if (file2->status == MBNA_FILE_FIXED)
			    	{
				xx[3*nc1]   += -weight * offsetsigma * tie->sigmax3[0];
				xx[3*nc1+1] += -weight * offsetsigma * tie->sigmax3[1];
				xx[3*nc1+2] += -weight * offsetsigma * tie->sigmax3[2];
				}
			    else
			    	{
				xx[3*nc1]   += -0.5 * weight * offsetsigma * tie->sigmax3[0];
				xx[3*nc1+1] += -0.5 * weight * offsetsigma * tie->sigmax3[1];
				xx[3*nc1+2] += -0.5 * weight * offsetsigma * tie->sigmax3[2];
				xx[3*nc2]   +=  0.5 * weight * offsetsigma * tie->sigmax3[0];
				xx[3*nc2+1] +=  0.5 * weight * offsetsigma * tie->sigmax3[1];
				xx[3*nc2+2] +=  0.5 * weight * offsetsigma * tie->sigmax3[2];
				}
/*fprintf(stderr,"semi-vertical:  nc1:%d xx:%f %f %f  nc2:%d xx:%f %f %f\n",
nc1,xx[3*nc1],xx[3*nc1+1],xx[3*nc1+2],
nc2,xx[3*nc2],xx[3*nc2+1],xx[3*nc2+2]);
			    
fprintf(stderr,"icrossing:%d j:%d tie->offset_x_m:%f x[3*%d]:%f x[3*%d]:%f offsetx:%f xx[3*%d]:%f xx[3*%d]:%f\n",
icrossing,j,tie->offset_x_m,nc2,x[3*nc2],nc1,x[3*nc1],offsetx,nc2,xx[3*nc2],nc1,xx[3*nc1]);
fprintf(stderr,"icrossing:%d j:%d tie->offset_y_m:%f x[3*%d+1]:%f x[3*%d+1]:%f offsety:%f xx[3*%d+1]:%f xx[3*%d+1]:%f\n",
icrossing,j,tie->offset_y_m,nc2,x[3*nc2+1],nc1,x[3*nc1+1],offsety,nc2,xx[3*nc2+1],nc1,xx[3*nc1+1]);
fprintf(stderr,"icrossing:%d j:%d tie->offset_z_m:%f x[3*%d+2]:%f x[3*%d+2]:%f offsetz:%f xx[3*%d+2]:%f xx[3*%d+2]:%f\n\n",
icrossing,j,tie->offset_z_m,nc2,x[3*nc2+2],nc1,x[3*nc1+2],offsetz,nc2,xx[3*nc2+2],nc1,xx[3*nc1+2]);*/
			    }
			}
			    
		    /* now loop over all points applying smoothing */
		    nseq = 0;
		    ndx = 0;
		    ndx2 = 0;
		    file = &project.files[ifile];
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			for (isection=0;isection<file->num_sections;isection++)
			    {
			    section = &file->sections[isection];
			    if (section->continuity == MB_NO)
			    	nseq = 0;
			    for (isnav=0;isnav<section->num_snav;isnav++)
				{
				/* work only with points that are tied */
				if (section->snav_num_ties[isnav] > 0)
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
					offsetx = (x[3*nc3]   + xx[3*nc3]   - x[3*nc2]   - xx[3*nc2]);
					offsety = (x[3*nc3+1] + xx[3*nc3+1] - x[3*nc2+1] - xx[3*nc2+1]);
					offsetz = (x[3*nc3+2] + xx[3*nc3+2] - x[3*nc2+2] - xx[3*nc2+2]);

					/* add remaining offsets to both points, or just one if one is fixed */
					weight = MIN(mbna_smoothweight / dtime_d, 1.0);
					xx[3*nc2]   +=  0.5 * weight * offsetx;
					xx[3*nc2+1] +=  0.5 * weight * offsety;
					xx[3*nc2+2] +=  0.5 * weight * offsetz;
					xx[3*nc3]   += -0.5 * weight * offsetx;
					xx[3*nc3+1] += -0.5 * weight * offsety;
					xx[3*nc3+2] += -0.5 * weight * offsetz;
					    
					ndx++;
/*fprintf(stderr,"1st Derivative: nc2:%d offsets:%f %f %f  weight:%f perturbation:%f %f %f\n",
nc2,offsetx,offsety,offsetz,weight,
0.5 * weight * offsetx,0.5 * weight * offsety,0.5 * weight * offsetz);*/
					}

				    /* add second derivative constraint if nseq > 2  AND dtime_d > 0.0 */
				    dtime_d = time_d3 - time_d1;
				    if (nseq > 1 && dtime_d > 0.0)
					{
			    		/* get current offset vector */
					offsetx = (x[3*nc1] + xx[3*nc1] 
							- 2.0 * (x[3*nc2] + xx[3*nc2]) 
							+ x[3*nc3] + xx[3*nc3]);
					offsety = (x[3*nc1+1] + xx[3*nc1+1] 
							- 2.0 * (x[3*nc2+1] + xx[3*nc2+1]) 
							+ x[3*nc3+1] + xx[3*nc3+1]);
					offsetz = (x[3*nc1+2] + xx[3*nc1+2] 
							- 2.0 * (x[3*nc2+2] + xx[3*nc2+2]) 
							+ x[3*nc3+2] + xx[3*nc3+2]);

					/* add remaining offsets to both points, or just one if one is fixed */	
					weight = MIN(mbna_smoothweight / dtime_d, 1.0);
					xx[3*nc1]   += -0.25 * weight * offsetx;
					xx[3*nc1+1] += -0.25 * weight * offsety;
					xx[3*nc1+2] += -0.25 * weight * offsetz;
					xx[3*nc2]   +=  0.50 * weight * offsetx;
					xx[3*nc2+1] +=  0.50 * weight * offsety;
					xx[3*nc2+2] +=  0.50 * weight * offsetz;
					xx[3*nc3]   += -0.25 * weight * offsetx;
					xx[3*nc3+1] += -0.25 * weight * offsety;
					xx[3*nc3+2] += -0.25 * weight * offsetz;
/*fprintf(stderr,"2nd Derivative: nc2:%d offsets:%f %f %f  weight:%f perturbation:%f %f %f\n",
nc2,offsetx,offsety,offsetz,weight,0.25 * weight * offsetx,0.25 * weight * offsety,0.25 * weight * offsetz);*/
					    
					ndx2++;
					}
				    
				    /* increment sequence counter */
				    nseq++;
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
/*fprintf(stderr,"i:%d x:%f ",i,x[i]);*/
			x[i] += xx[i];
/*fprintf(stderr,"xx:%f x:%f\n",xx[i],x[i]);*/
			}

		    /* calculate weighted misfit */
		    misfit_ties = 0.0;
		    ntie = 0;
		    for (icrossing=0;icrossing<project.num_crossings;icrossing++)
			{
			crossing = &project.crossings[icrossing];

			/* use only set crossings */
			if (crossing->status == MBNA_CROSSING_STATUS_SET)
			for (j=0;j<crossing->num_ties;j++)
			    {
			    /* get tie */
			    tie = (struct mbna_tie *) &crossing->ties[j];
			    ntie++;

			    /* get absolute id for first snav point */
			    file1 = &project.files[crossing->file_id_1];
			    section = &file1->sections[crossing->section_1];
			    nc1 = section->snav_invert_id[tie->snav_1];

			    /* get absolute id for second snav point */
			    file2 = &project.files[crossing->file_id_2];
			    section = &file2->sections[crossing->section_2];
			    nc2 = section->snav_invert_id[tie->snav_2];
if (nc1 > nsnav - 1 || nc2 > nsnav -1
|| nc1 < 0 || nc2 < 0)
fprintf(stderr, "BAD snav ID: %d %d %d\n", nc1, nc2, nsnav);

			    /* get observed offset vector including reduction of block solution */
			    offsetx = tie->offset_x_m - file2->block_offset_x + file1->block_offset_x
			    		- (x[3*nc2] - x[3*nc1]);
			    offsety = tie->offset_y_m - file2->block_offset_y + file1->block_offset_y
			    		- (x[3*nc2+1] - x[3*nc1+1]);
			    offsetz = tie->offset_z_m - file2->block_offset_z + file1->block_offset_z
			    		- (x[3*nc2+2] - x[3*nc1+2]);

			    /* get long axis misfit */
			    misfit = mbna_offsetweight / tie->sigmar1
						    * (offsetx * tie->sigmax1[0]
							    + offsety * tie->sigmax1[1]
							    + offsetz * tie->sigmax1[2]);
			    misfit_ties += misfit * misfit;

			    /* get horizontal axis misfit */
			    misfit = mbna_offsetweight / tie->sigmar2
						    * (offsetx * tie->sigmax2[0]
							    + offsety * tie->sigmax2[1]
							    + offsetz * tie->sigmax2[2]);
			    misfit_ties += misfit * misfit;

			    /* get semi-vertical axis misfit */
			    misfit = mbna_offsetweight / tie->sigmar3
						    * (offsetx * tie->sigmax3[0]
							    + offsety * tie->sigmax3[1]
							    + offsetz * tie->sigmax3[2]);
			    misfit_ties += misfit * misfit;
			    }
			}
		    misfit_ties = sqrt(misfit_ties) / ntie;
		    
		    iter++;
		    
/* fprintf(stderr,"iter:%d ntie:%d misfit_initial:%f misfit_ties:%f perturbationsize:%f\n",
iter,ntie,misfit_initial,misfit_ties,perturbationsize);*/

		    /* check for convergence */
		    if (perturbationsize / misfit_initial < 0.00000001 || iter > ncols)
		    	done = MB_YES;

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
				if (section->snav_num_ties[isnav] > 0)
				    {
				    k = section->snav_invert_id[isnav];
				    section->snav_lon_offset[isnav] = (x[3*k] + file->block_offset_x) * mbna_mtodeglon;
				    section->snav_lat_offset[isnav] = (x[3*k+1] + file->block_offset_y) * mbna_mtodeglat;
				    section->snav_z_offset[isnav] = (x[3*k+2] + file->block_offset_z);
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
		    if (project.modelplot == MB_YES && iter % 500 == 0)
		    	mbnavadjust_modelplot_plot();
		    }
		}
		
	/* output results from navigation solution */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed == project.num_crossings
			|| project.num_truecrossings_analyzed == project.num_truecrossings)
		&& error == MB_ERROR_NO_ERROR)
    		{
		
		/* now output inverse solution */
		sprintf(message,"Outputting navigation solution...");
		do_message_on(message);

		sprintf(message, " > Final smoothing weight:%12g\n > Final crossing sigma:%12g\n > Final total sigma:%12g\n",
			smoothweight_best, sigma_crossing_best, sigma_total_best);
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

		if (mbna_verbose > 0)
		for (i=0;i<nc/3;i++)
		    {
		    fprintf(stderr, "i:%d  offsets: %f %f %f\n",
		    		i, xx[3*i], xx[3*i+1], xx[3*i+2]);
		    }
		
		/* write updated project */
		project.inversion = MBNA_INVERSION_CURRENT;
		mbnavadjust_write_project();

		/* deallocate arrays */
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&xx,&error);
		
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
	struct mbna_file *file1;
	struct mbna_file *file2;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
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
	double	time_d1, time_d2, time_d3;
	int	done;
	int	isection, isnav;
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
		&& (project.num_crossings_analyzed == project.num_crossings
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
		    if (file->status != MBNA_FILE_FIXED)
		    	{
		    	sprintf(npath,"%s/nvs_%4.4d.mb166",
			    project.datadir,i);
		    	sprintf(apath,"%s/nvs_%4.4d.na%d",
			    project.datadir,i,file->output_id);
		    	sprintf(opath,"%s.na%d", file->path, file->output_id);
		    	if ((nfp = fopen(npath,"r")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		    	else if ((afp = fopen(apath,"w")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		   	else if ((ofp = fopen(opath,"w")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		    	else
			    {
			    sprintf(message, " > Output updated nav to %s\n", opath);
			    do_info_add(message, MB_NO);

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
			    	else if ((nscan = sscanf(buffer, "%d %d %d %d %d %d.%d %lf %lf %lf %lf %lf %lf %lf %lf %lf",
							&time_i[0], &time_i[1], &time_i[2], &time_i[3],
							&time_i[4], &time_i[5], &time_i[6], &time_d,
							&navlon, &navlat, &heading, &speed, 
							&draft, &roll, &pitch, &heave)) >= 12)
				    {
				    /* fix nav from early version */
				    if (nscan < 16)
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
					navlon -= section->snav_lon_offset[isnav]
							+ factor * (section->snav_lon_offset[isnav+1]
								- section->snav_lon_offset[isnav]);
					navlat -= section->snav_lat_offset[isnav]
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
					sprintf(ostring, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						    time_i[0], time_i[1], time_i[2], time_i[3],
						    time_i[4], time_i[5], time_i[6], time_d,
						    navlon, navlat, heading, speed, 
						    draft, roll, pitch, heave, zoffset);
					sprintf(ostring, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						    time_i[0], time_i[1], time_i[2], time_i[3],
						    time_i[4], time_i[5], time_i[6], time_d,
						    navlon, navlat, heading, speed, 
						    draft, roll, pitch, heave, zoffset);
					fprintf(ofp, "%s", ostring);
					fprintf(afp, "%s", ostring);
/*fprintf(stderr, "%2.2d:%2.2d:%2.2d:%5.3f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f\n\n",
i, isection, isnav, factor, 
time_i[0], time_i[1], time_i[2], time_i[3],
time_i[4], time_i[5], time_i[6], time_d,
navlon, navlat, heading, speed, zoffset);*/
					}
				    }
			    	}
			    }
		    	if (nfp != NULL) fclose(nfp);
		    	if (afp != NULL) fclose(afp);
		    	if (ofp != NULL) 
			    {
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
mbnavadjust_invertnav0()
{
	/* local variables */
	char	*function_name = "mbnavadjust_invertnav0";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_file *file1;
	struct mbna_file *file2;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	nnav, nsnav, nfix, ndx, ndx2, ntie, nconstraint;
	int	nblock;
	int	nseq;
	int	nnz;
	int	ndf = 3;
	int	nrows = 0;
	int	ncols = 0;
	double	*a;
	int	*ia;
	int	*nia;
	int	*nx;
	double	*x;
	double	*dx;
	double	*d;
	double	*w;
	double	*sigma;
	double	*work;
	double	time_d, time_d_old, time_d_older, dtime_d;
	double	avg_dtime_d, avg_offset;
	int	ifile, icrossing, isection, isnav;
	int	nr, nc, nc1, nc2;
	int	ncyc, nsig;
	double	smax, sup, err, supt, slo, errlsq, s;
	int	ncycle = 2048;
	double	bandwidth = 10000.0;
	double	smoothweight, sigma_total, sigma_crossing;
	double	sigma_crossing_first;
	double	smoothweight_old, smoothweight_best, sigma_total_best, sigma_crossing_best;
	double	smoothfactor, smoothmin, smoothmax;
	double	offset_x, offset_y, offset_z;
	int	first;
	int	iter;
	char	npath[STRING_MAX];
	char	apath[STRING_MAX];
	char	opath[STRING_MAX];
	char	ostring[STRING_MAX];
	FILE	*nfp, *afp, *ofp;
	char	*result;
	char	buffer[BUFFER_MAX];
	int	done, nscan;
	double	factor;
	int	time_i[7];
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	double	zoffset;
	int	mbp_heading_mode;
	double	mbp_headingbias;
	int	mbp_rollbias_mode;
	double	mbp_rollbias;
	double	mbp_rollbias_port;
	double	mbp_rollbias_stbd;
	double	fixed_block_offset_x;
	double	fixed_block_offset_y;
	double	fixed_block_offset_z;
	int	nfixed_block_offset;
	double	offsetx, offsety, offsetz;
	double	kluge;
	int	i, j, k;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* invert if there is a project and all crossings have been analyzed */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed == project.num_crossings
			|| project.num_truecrossings_analyzed == project.num_truecrossings))
			
    		{
		/* set message dialog on */
		sprintf(message,"Setting up navigation inversion...");
		do_message_on(message);
		
		/*----------------------------------------------------------------*/
		
		/* figure out the average offsets between connected sets of files 
			- invert for x y z offsets for the blocks */
			
		/* count the number of blocks */
		nblock = 0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    if (i==0 || file->sections[0].continuity == MB_NO)
		    	{
			nblock++;
			}
		    file->block = nblock - 1;
		    file->block_offset_x = 0.0;
		    file->block_offset_y = 0.0;
		    file->block_offset_z = 0.0;
		    }
		ntie = 0;
		for (icrossing=0;icrossing<project.num_crossings;icrossing++)
		    {
		    crossing = &project.crossings[icrossing];
		    if (crossing->status == MBNA_CROSSING_STATUS_SET)
		    ntie += crossing->num_ties;
		    }
		
		/* invert for block offsets only if there is more than one block */
		if (nblock > 1)
		    {
		    /* allocate space for the inverse problem */
		    nconstraint = ndf * ntie;
		    nrows = nconstraint;
		    ncols = ndf * nblock;
		    nnz = 3;
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnz * nrows * sizeof(double), (void **)&a,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnz * nrows * sizeof(int), (void **)&ia,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows * sizeof(int), (void **)&nia,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows * sizeof(double), (void **)&d,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&x,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(int), (void **)&nx,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&dx,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncycle * sizeof(double), (void **)&sigma,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncycle * sizeof(double), (void **)&work,&error);

		    /* initialize arrays */
		    for (i=0;i<nrows;i++)
			{
			nia[i] = 0;
			d[i] = 0.0;
			for (j=0;j<nnz;j++)
			    {
			    k = nnz * i + j;
			    ia[k] = 0;
			    a[k] = 0.0;
			    }
			}
		    for (i=0;i<ndf*nblock;i++)
			{
			nx[i] = 0;
			x[i] = 0;
			dx[i] = 0.0;
			}
		    for (i=0;i<ncycle;i++)
			{
			sigma[i] = 0;
			work[i] = 0.0;
			}

		    /* construct the inverse problem */
		    sprintf(message,"Solving for block offsets...");
		    do_message_on(message);
		    
		    /* loop over crossings getting set ties */
		    nr = 0;
		    nc = ndf * nblock;
		    ntie = 0;
		    for (icrossing=0;icrossing<project.num_crossings;icrossing++)
			{
			crossing = &project.crossings[icrossing];

			/* use only set crossings */
			if (crossing->status == MBNA_CROSSING_STATUS_SET)
			for (j=0;j<crossing->num_ties;j++)
			    {
			    /* get tie */
			    tie = (struct mbna_tie *) &crossing->ties[j];
			    ntie++;

			    /* get block id for first snav point */
			    file = &project.files[crossing->file_id_1];
			    nc1 = file->block;

			    /* get block id for second snav point */
			    file = &project.files[crossing->file_id_2];
			    nc2 = file->block;

			    /* make longitude constraint */
			    k = nnz * nr;
			    a[k] = -1.0;
			    a[k+1] = 1.0;
			    d[nr] = tie->offset_x_m;
			    ia[k] = 3 * nc1;
			    ia[k+1] = 3 * nc2;
			    nia[nr] = 2;
			    nx[3 * nc1]++;
			    nx[3 * nc2]++;
/* fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%g\n", nr,k, a[k],a[k+1],d[nr]); */
			    nr++;

			    /* make latitude constraint */
			    k = nnz * nr;
			    a[k] = -1.0;
			    a[k+1] = 1.0;
			    d[nr] = tie->offset_y_m;
			    ia[k] = 3 * nc1 + 1;
			    ia[k+1] = 3 * nc2 + 1;
			    nia[nr] = 2;
			    nx[3 * nc1 + 1]++;
			    nx[3 * nc2 + 1]++;
/* fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%g\n", nr,k, a[k],a[k+1],d[nr]); */
			    nr++;

			    /* make depth constraint */
			    k = nnz * nr;
			    a[k] = -1.0;
			    a[k+1] = 1.0;
			    d[nr] = tie->offset_z_m;
			    ia[k] = 3 * nc1 + 2;
			    ia[k+1] = 3 * nc2 + 2;
			    nia[nr] = 2;
			    nx[3 * nc1 + 2]++;
			    nx[3 * nc2 + 2]++;
/* fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%g\n", nr,k, a[k],a[k+1],d[nr]); */
			    nr++;
			    }
			}

		    /* solve the inverse problem */

		    /* compute upper bound on maximum eigenvalue */
		    ncyc = 0;
		    nsig = 0;
		    lspeig(a, ia, nia, nnz, nc, nr, ncyc,
			    &nsig, x, dx, sigma, work, &smax, &err, &sup);
		    supt = smax + err;
		    if (sup > supt)
			supt = sup;
		    if (mbna_verbose > 0)
		    fprintf(stderr, "Initial lspeig: sup:%g smax:%g err:%g supt:%g\n",
			sup, smax, err, supt);
		    ncyc = 16;
		    for (i=0;i<4;i++)
			{
			lspeig(a, ia, nia, nnz, nc, nr, ncyc,
				&nsig, x, dx, sigma, work, &smax, &err, &sup);
			supt = smax + err;
			if (sup > supt)
			    supt = sup;
			if (mbna_verbose > 0)
			fprintf(stderr, "lspeig[%d]: sup:%g smax:%g err:%g supt:%g\n",
			    i, sup, smax, err, supt);
			}

		    /* calculate chebyshev factors (errlsq is the theoretical error) */
		    slo = supt / bandwidth;
		    chebyu(sigma, ncycle, supt, slo, work);
		    errlsq = errlim(sigma, ncycle, supt, slo);
		    if (mbna_verbose > 0)
		    fprintf(stderr, "Theoretical error: %f\n", errlsq);
		    if (mbna_verbose > 1)
		    for (i=0;i<ncycle;i++)
			fprintf(stderr, "sigma[%d]: %f\n", i, sigma[i]);

		    /* solve the problem */
		    for (i=0;i<nc;i++)
			x[i] = 0.0;
		    lsqup(a, ia, nia, nnz, nc, nr, x, dx, d, 0, NULL, NULL, ncycle, sigma);

		    /* output solution */
		    if (mbna_verbose > 1)
		    for (i=0;i<nc/3;i++)
			{
			fprintf(stderr, "block:%d  offsets: %f %f %f  nties: %d %d %d\n",
				i, x[3*i], x[3*i+1], x[3*i+2], nx[3*i], nx[3*i+1], nx[3*i+2]);
			}

		    /* calculate error */
		    sigma_total = 0.0;
		    sigma_crossing = 0.0;
		    for (i=0;i<nr;i++)
			{
			s = 0.0;
			for (j=0;j<nia[i];j++)
			    {
			    k = nnz * i + j;
			    s += x[ia[k]] * a[k];
			    }
			sigma_total += (d[i] - s) * (d[i] - s);
			if (i >= (nr - 3 * ntie))
			    sigma_crossing += (d[i] - s) * (d[i] - s);
			}
		    sigma_total = sqrt(sigma_total) / nr;
		    sigma_crossing = sqrt(sigma_crossing) / ntie;

		    /* extract results */
		    fixed_block_offset_x = 0.0;
		    fixed_block_offset_y = 0.0;
		    fixed_block_offset_z = 0.0;
		    nfixed_block_offset = 0;
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			if (file->status == MBNA_FILE_FIXED)
				{
				fixed_block_offset_x -= x[3 * file->block];
				fixed_block_offset_y -= x[3 * file->block + 1];
				fixed_block_offset_z -= x[3 * file->block + 2];
				nfixed_block_offset++;
				}
			file->block_offset_x = x[3 * file->block];
			file->block_offset_y = x[3 * file->block + 1];
			file->block_offset_z = x[3 * file->block + 2];
			}
		    if (nfixed_block_offset > 0)
		        {
			fixed_block_offset_x /= (double)nfixed_block_offset;
			fixed_block_offset_y /= (double)nfixed_block_offset;
			fixed_block_offset_z /= (double)nfixed_block_offset;
			for (i=0;i<project.num_files;i++)
			    {
			    file = &project.files[i];
			    file->block_offset_x += fixed_block_offset_x;
			    file->block_offset_y += fixed_block_offset_y;
			    file->block_offset_z += fixed_block_offset_z;
			    }
			}
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
fprintf(stderr,"file:%d block: %d block offsets: %f %f %f\n",
i,file->block,file->block_offset_x,file->block_offset_y,file->block_offset_z);
			}

		    /* deallocate arrays */
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&a,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ia,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nia,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&d,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nx,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&dx,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&sigma,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&work,&error);
		    }
		
		/*----------------------------------------------------------------*/		

     		/* retrieve crossing parameters */
		/* count up the unknowns and constraints to get size of
		 * inverse problem:
		 *	    nconstraint = ndf * nsnav + ntie
		 */
		nnav = 0;
		nsnav = 0;
		nfix = 0;
		ntie = 0;
		nseq = 0;
		ndx = 0;
		ndx2 = 0;
		avg_dtime_d = 0.0;
		for (i=0;i<project.num_files;i++)
		    {
sprintf(message,"Processing file %d of %d...", i, project.num_files);
do_message_on(message);
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			nnav += section->num_pings;
			nsnav += section->num_snav - section->continuity;
			if (file->status == MBNA_FILE_FIXED)
			    nfix += section->num_snav - section->continuity;
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    if (isnav > 0 || section->continuity == MB_NO)
				{
				/* handle sequence count */
				if (isnav > 0)
				    nseq++;
				else
				    nseq = 1;
				    
				/* get time difference between current and previous nav point */
				dtime_d = section->snav_time_d[isnav] - time_d_old;
				
				/* add first derivative constraint if nseq > 1 AND dtime_d > 0.0 */
				if (nseq > 1 && dtime_d > 0.0)
				    {
				    avg_dtime_d += dtime_d;
				    ndx++;
				    }
				
				/* add second derivative constraint if nseq > 2 */
				if (nseq > 2 && dtime_d > 0.0)
				    ndx2++;
				
				/* save time_d values */
				time_d_old = section->snav_time_d[isnav];
				}
			    }
			}
		    }
		if (ndx > 0)
		    avg_dtime_d /= ndx;
		avg_offset = 0.0;
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &project.crossings[i];
		    if (crossing->status == MBNA_CROSSING_STATUS_SET
		    	&& crossing->num_ties > 0)
			{
			ntie += crossing->num_ties;
			for (j=0;j<crossing->num_ties;j++)
			    {
			    avg_offset += fabs(crossing->ties[j].offset_x);
			    }
			}
		    }
		if (ntie > 0)
		    avg_offset /= ntie;
		
		/* allocate space for the inverse problem */
		nconstraint = ndf * (nfix + ndx + ndx2 + ntie);
		nrows = nconstraint;
		ncols = ndf * nsnav;
		nnz = 6;
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnz * nrows * sizeof(double), (void **)&a,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnz * nrows * sizeof(int), (void **)&ia,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows * sizeof(int), (void **)&nia,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows * sizeof(double), (void **)&d,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows * sizeof(double), (void **)&w,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&x,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(int), (void **)&nx,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&dx,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncycle * sizeof(double), (void **)&sigma,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncycle * sizeof(double), (void **)&work,&error);

		/* if error initializing memory then don't invert */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(mbna_verbose,error,&error_message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
				error_message);
			}
		}
		
	/* proceed with construction of inverse problems */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed == project.num_crossings
			|| project.num_truecrossings_analyzed == project.num_truecrossings)
		&& error == MB_ERROR_NO_ERROR)
    		{
		/* add info text */
		sprintf(message, "Inverting for optimal navigation\n");
		do_info_add(message, MB_YES);
		sprintf(message, " > Inverse problem size:\n");
		do_info_add(message, MB_NO);
		sprintf(message, " >   Nav points:                    %d\n", nnav);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Independent nav snav points:   %d\n", nsnav);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Fixed nav snav points:         %d\n", nfix);
		do_info_add(message, MB_NO);
		sprintf(message, " >   First derivative constraints:  %d\n", ndx);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Second derivative constraints: %d\n", ndx2);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Nav ties:                      %d\n", ntie);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Total Unknowns:                %d\n", 3 * nsnav);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Total Constraints:             %d\n", nconstraint);
		do_info_add(message, MB_NO);
		sprintf(message, " > Iteration Smoothing S_total S_crossing Ratio\n > --------------------------------------\n");
		do_info_add(message, MB_NO);
		
		/* initialize arrays */
		for (i=0;i<nrows;i++)
		    {
		    nia[i] = 0;
		    d[i] = 0.0;
		    w[i] = 0.0;
		    for (j=0;j<nnz;j++)
			{
			k = nnz * i + j;
			ia[k] = 0;
			a[k] = 0.0;
			}
		    }
		for (i=0;i<ndf*nsnav;i++)
		    {
		    nx[i] = 0;
		    x[i] = 0;
		    dx[i] = 0.0;
		    }
		for (i=0;i<ncycle;i++)
		    {
		    sigma[i] = 0;
		    work[i] = 0.0;
		    }
		
		/* make inverse problem */
		/*------------------------*/
		nr = 0;
		smoothweight = avg_dtime_d * avg_offset / 100000000;
		smoothweight_old = smoothweight;
		
		/* apply first and second derivative constraints */
		nc = 0;
		nseq = 0;
		for (ifile=0;ifile<project.num_files;ifile++)
		    {
sprintf(message,"Applying derivatives to file %d of %d...", ifile, project.num_files);
do_message_on(message);
		    file = &project.files[ifile];
		    for (isection=0;isection<file->num_sections;isection++)
			{
			section = &file->sections[isection];
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {			
			    /* if continuity true first snav point is same as the last
			       snav point of previous section */
			    if (isnav > 0 || section->continuity == MB_NO)
				{
				/* handle sequence count */
				if (isnav > 0)
				    nseq++;
				else
				    nseq = 1;
				    
			    	/* if file fixed then fix snav point */
			    	if (file->status == MBNA_FILE_FIXED)
				    {
				    /* longitude component */
				    k = nnz * nr;
				    a[k] = 1.0;
				    d[nr] = 0.0;
				    w[nr] = 1.0;
				    ia[k] = nc;
				    nia[nr] = 1;
				    nr++;
				
				    /* latitude component */
				    k = nnz * nr;
				    a[k] = 1.0;
				    d[nr] = 0.0;
				    w[nr] = 1.0;
				    ia[k] = nc + 1;
				    nia[nr] = 1;
				    nr++;
				
				    /* depth component */
				    k = nnz * nr;
				    a[k] = 100.0;
				    d[nr] = 0.0;
				    w[nr] = 100.0;
				    ia[k] = nc + 2;
				    nia[nr] = 1;
				    nr++;
				    }
				    
				/* get time difference between current and previous nav point */
				dtime_d = section->snav_time_d[isnav] - time_d_old;
				
				/* add first derivative constraint if nseq > 1 AND dtime_d > 0.0 */
				if (nseq > 1 && dtime_d > 0.0)
				    {
				    /* longitude component */
				    k = nnz * nr;
				    a[k] = -0.01 *smoothweight_old / dtime_d;
				    a[k+1] = 0.01 *smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    w[nr] = 0.01 *smoothweight_old;
				    ia[k] = nc - 3;
				    ia[k+1] = nc;
				    nia[nr] = 2;
				    nr++;
				
				    /* latitude component */
				    k = nnz * nr;
				    a[k] = -0.01 *smoothweight_old / dtime_d;
				    a[k+1] = 0.01 *smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    w[nr] = 0.01 *smoothweight_old;
				    ia[k] = nc - 2;
				    ia[k+1] = nc + 1;
				    nia[nr] = 2;
				    nr++;
				
				    /* depth component */
				    k = nnz * nr;
				    a[k] = -0.01 *smoothweight_old / dtime_d;
				    a[k+1] = 0.01 *smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    w[nr] = 0.01 *smoothweight_old;
				    ia[k] = nc - 1;
				    ia[k+1] = nc + 2;
				    nia[nr] = 2;
				    nr++;
				    }
				
				/* add second derivative constraint if nseq > 2  AND dtime_d > 0.0 */
				if (nseq > 2 && dtime_d > 0.0)
				    {
				    /* longitude component */
				    k = nnz * nr;
				    a[k] = smoothweight_old / dtime_d;
				    a[k+1] = -2.0 * smoothweight_old / dtime_d;
				    a[k+2] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    w[nr] = smoothweight_old;
				    ia[k] = nc - 6;
				    ia[k+1] = nc - 3;
				    ia[k+2] = nc;
				    nia[nr] = 3;
				    nr++;
				
				    /* latitude component */
				    k = nnz * nr;
				    a[k] = smoothweight_old / dtime_d;
				    a[k+1] = -2.0 * smoothweight_old / dtime_d;
				    a[k+2] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    w[nr] = smoothweight_old;
				    ia[k] = nc - 5;
				    ia[k+1] = nc - 2;
				    ia[k+2] = nc + 1;
				    nia[nr] = 3;
				    nr++;
				
				    /* depth component */
				    k = nnz * nr;
				    a[k] = smoothweight_old / dtime_d;
				    a[k+1] = -2.0 * smoothweight_old / dtime_d;
				    a[k+2] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    w[nr] = smoothweight_old;
				    ia[k] = nc - 4;
				    ia[k+1] = nc - 1;
				    ia[k+2] = nc + 2;
				    nia[nr] = 3;
				    nr++;
				    }
				
				/* save time_d values */
				time_d_older = time_d_old;
				time_d_old = section->snav_time_d[isnav];

				/* add to unknown count */
				nc += ndf;
				}
			    }
			}
		    }
		
		/* apply crossing offset constraints */
sprintf(message,"Applying %d crossing constraints...", project.num_crossings);
do_message_on(message);
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
			section = &file1->sections[crossing->section_1];
			nc1 = section->global_start_snav + tie->snav_1;

			/* get absolute id for second snav point */
			file2 = &project.files[crossing->file_id_2];
			section = &file2->sections[crossing->section_2];
			nc2 = section->global_start_snav + tie->snav_2;
if (nc1 > nsnav - 1 || nc2 > nsnav -1
|| nc1 < 0 || nc2 < 0)
fprintf(stderr, "BAD snav ID: %d %d %d\n", nc1, nc2, nsnav);

			/* get observed offset vector including reduction of block solution */
			offsetx = tie->offset_x_m - file2->block_offset_x + file1->block_offset_x;
			offsety = tie->offset_y_m - file2->block_offset_y + file1->block_offset_y;
			offsetz = tie->offset_z_m - file2->block_offset_z + file1->block_offset_z;
			
			/* make long axis constraint */
			k = nnz * nr;
			w[nr] = mbna_offsetweight / tie->sigmar1;
			a[k] = -w[nr] * tie->sigmax1[0];
			a[k+1] = w[nr] * tie->sigmax1[0];
			a[k+2] = -w[nr] * tie->sigmax1[1];
			a[k+3] = w[nr] * tie->sigmax1[1];
			a[k+4] = -w[nr] * tie->sigmax1[2];
			a[k+5] = w[nr] * tie->sigmax1[2];
			d[nr] = w[nr] * (offsetx * tie->sigmax1[0]
							+ offsety * tie->sigmax1[1]
							+ offsetz * tie->sigmax1[2]);
			ia[k] = 3 * nc1;
			ia[k+1] = 3 * nc2;
			ia[k+2] = 3 * nc1 + 1;
			ia[k+3] = 3 * nc2 + 1;
			ia[k+4] = 3 * nc1 + 2;
			ia[k+5] = 3 * nc2 + 2;
			nia[nr] = 6;
/*fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%d\n", nr,k, a[k],a[k+1],d[nr]);*/
			nr++;
			
			/* make horizontal axis constraint */
			k = nnz * nr;
			w[nr] = mbna_offsetweight / tie->sigmar2;
			a[k] = -w[nr] * tie->sigmax2[0];
			a[k+1] = w[nr] * tie->sigmax2[0];
			a[k+2] = -w[nr] * tie->sigmax2[1];
			a[k+3] = w[nr] * tie->sigmax2[1];
			a[k+4] = -w[nr] * tie->sigmax2[2];
			a[k+5] = w[nr] * tie->sigmax2[2];
			d[nr] = w[nr] * (offsetx * tie->sigmax2[0]
							+ offsety * tie->sigmax2[1]
							+ offsetz * tie->sigmax2[2]);
			ia[k] = 3 * nc1;
			ia[k+1] = 3 * nc2;
			ia[k+2] = 3 * nc1 + 1;
			ia[k+3] = 3 * nc2 + 1;
			ia[k+4] = 3 * nc1 + 2;
			ia[k+5] = 3 * nc2 + 2;
			nia[nr] = 6;
/*fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%d\n", nr,k, a[k],a[k+1],d[nr]);*/
			nr++;
			
			/* make semi-vertical constraint */
			k = nnz * nr;
			kluge = 1.0;
			w[nr] = kluge * mbna_offsetweight / tie->sigmar3;
			a[k] = -w[nr] * tie->sigmax3[0];
			a[k+1] = w[nr] * tie->sigmax3[0];
			a[k+2] = -w[nr] * tie->sigmax3[1];
			a[k+3] = w[nr] * tie->sigmax3[1];
			a[k+4] = -w[nr] * tie->sigmax3[2];
			a[k+5] = w[nr] * tie->sigmax3[2];
			d[nr] = w[nr] * (offsetx * tie->sigmax3[0]
							+ offsety * tie->sigmax3[1]
							+ offsetz * tie->sigmax3[2]);
			ia[k] = 3 * nc1;
			ia[k+1] = 3 * nc2;
			ia[k+2] = 3 * nc1 + 1;
			ia[k+3] = 3 * nc2 + 1;
			ia[k+4] = 3 * nc1 + 2;
			ia[k+5] = 3 * nc2 + 2;
			nia[nr] = 6;
			
			/* count the tie */
			nx[3 * nc1]++;
			nx[3 * nc2]++;
			nx[3 * nc1 + 1]++;
			nx[3 * nc2 + 1]++;
			nx[3 * nc1 + 2]++;
			nx[3 * nc2 + 2]++;
/*fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%d\n", nr,k, a[k],a[k+1],d[nr]);*/
			nr++;
			}
		    }
		    
/* print out matrix problem & calculate error */
/*sigma_total = 0.0;
sigma_crossing = 0.0;
fprintf(stderr,"START PROBLEM: smoothweight:%g avg_dtime_d:%g avg_offset:%g\n",
smoothweight,avg_dtime_d,avg_offset);

for (i=0;i<nr;i++)
{
fprintf(stderr,"row %d with %d elements\n", i, nia[i]);
s = 0.0;
for (j=0;j<nia[i];j++)
    {
    k = nnz * i + j;
    s += x[ia[k]] * a[k];
fprintf(stderr,"element %d column %d    a:%g  x:%g  s:%g\n", j, ia[k], a[k], x[ia[k]], x[ia[k]] * a[k]);
    }
sigma_total += (d[i] - s) * (d[i] - s);
fprintf(stderr,"row %d result:  s:%g d:%g sigma_total:%g %g\n\n", i, s, d[i], (d[i] - s) * (d[i] - s), sigma_total);
if (i >= (nr - ndf * ntie))
    sigma_crossing += (d[i] - s) * (d[i] - s);
}
sigma_total = sqrt(sigma_total) / nr;
sigma_crossing = sqrt(sigma_crossing) / ntie;
fprintf(stderr,"INITIAL SIGMA: sigma_total:%g sigma_crossing:%g\n", sigma_total, sigma_crossing);
*/

		/* now do a bunch of test solutions to find what level of smoothing
		 * starts to impact the solution - we want to use this much and no more
		 */
		first = MB_YES;
		done = MB_NO;
		iter=0;
		smoothfactor = 100.0;
		smoothmin = -1.0;
		smoothmax = -1.0;
		while (done == MB_NO)
		    {
		    /* first change the smoothing weight in the matrix problem */
		    smoothweight = smoothweight * smoothfactor;
		    iter++;
		    for (i=0;i<ndf*(nfix+ndx+ndx2);i++)
			{
			for (j=0;j<nia[i];j++)
			    {
			    if (nia[i] > 1)
				{
				k = nnz * i + j;
				a[k] *= smoothfactor;
				}
			    }
			}
		
		    /* now solve inverse problem */
		    if (first == MB_YES)
			{
			sprintf(message,"Inverting %dx%d: iter:%d smooth:%.2g",
				nc, nr, iter, smoothweight);
			}
		    else
			{
			sprintf(message,"Inverting %dx%d: iter:%d smooth:%.2g ratio:%.3f iter:%d smooth:%.2g ",
				nc, nr, iter-1, smoothweight_old,
				(sigma_crossing / sigma_crossing_first), iter, smoothweight);
			}
		    do_message_on(message);
		
		    /* compute upper bound on maximum eigenvalue */
		    ncyc = 0;
		    nsig = 0;
		    lspeig(a, ia, nia, nnz, nc, nr, ncyc,
			    &nsig, x, dx, sigma, work, &smax, &err, &sup);
		    supt = smax + err;
		    if (sup > supt)
			supt = sup;
		    if (mbna_verbose > 0)
		    fprintf(stderr, "Initial lspeig: sup:%g smax:%g err:%g supt:%g\n",
			sup, smax, err, supt);
		    ncyc = 16;
		    for (i=0;i<4;i++)
			{
			lspeig(a, ia, nia, nnz, nc, nr, ncyc,
				&nsig, x, dx, sigma, work, &smax, &err, &sup);
			supt = smax + err;
			if (sup > supt)
			    supt = sup;
			if (mbna_verbose > 0)
			fprintf(stderr, "lspeig[%d]: sup:%g smax:%g err:%g supt:%g\n",
			    i, sup, smax, err, supt);
			}
			
		    /* calculate chebyshev factors (errlsq is the theoretical error) */
		    slo = supt / bandwidth;
		    chebyu(sigma, ncycle, supt, slo, work);
		    errlsq = errlim(sigma, ncycle, supt, slo);
		    if (mbna_verbose > 0)
		    fprintf(stderr, "Theoretical error: %f\n", errlsq);
		    if (mbna_verbose > 1)
		    for (i=0;i<ncycle;i++)
			fprintf(stderr, "sigma[%d]: %f\n", i, sigma[i]);
			
		    /* solve the problem */
		    for (i=0;i<nc;i++)
			x[i] = 0.0;
		    lsqup(a, ia, nia, nnz, nc, nr, x, dx, d, 0, NULL, NULL, ncycle, sigma);
	
		    /* output solution */
		    if (mbna_verbose > 1)
		    for (i=0;i<nc/3;i++)
			{
			fprintf(stderr, "i:%d  offsets: %f %f %f  nties: %d %d %d\n",
				i, x[3*i], x[3*i+1], x[3*i+2], nx[3*i], nx[3*i+1], nx[3*i+2]);
			}
		
		    /* calculate total error */
		    sigma_total = 0.0;
		    sigma_crossing = 0.0;
		    for (i=0;i<nr;i++)
			{
			s = 0.0;
			for (j=0;j<nia[i];j++)
			    {
			    k = nnz * i + j;
			    s += x[ia[k]] * a[k];
			    }
			sigma_total += (d[i] - s) * (d[i] - s);
			if (i >= (nr - ndf * ntie))
			    sigma_crossing += (d[i] - s) * (d[i] - s);
			}
		    sigma_total = sqrt(sigma_total) / nr;
		    sigma_crossing = sqrt(sigma_crossing) / ntie;
		
		    /* calculate tie error */
		    sigma_crossing = 0.0;
		    for (i=(nr - ndf * ntie);i<nr;i++)
			{
			s = 0.0;
			for (j=0;j<nia[i];j++)
			    {
			    k = nnz * i + j;
			    s += x[ia[k]] * a[k] / w[i];
			    }
			sigma_crossing += (d[i] / w[i] - s) * (d[i] / w[i] - s);
			}
		    sigma_crossing = sqrt(sigma_crossing) / ntie;
		
		    /* keep track of results */
		    if (first == MB_YES)
			{
			first = MB_NO;
			sigma_crossing_first = MAX(sigma_crossing, project.precision);
			smoothweight_old = smoothweight;
			smoothmin = smoothweight;
			}
		    else if (sigma_crossing >= 1.005 * sigma_crossing_first
			    && sigma_crossing <= 1.01 * sigma_crossing_first
			    && sigma_crossing > 0.0)
			{
			done = MB_YES;
			smoothweight_best = smoothweight;
			sigma_total_best = sigma_total;
			sigma_crossing_best = sigma_crossing;
			smoothweight_old = smoothweight;
			}
		    else if (sigma_crossing < 1.005 * sigma_crossing_first)
			{
			if (smoothweight > smoothmin)
			    {
			    smoothmin = smoothweight;
			    }
			if (smoothmax > 0.0)
			    smoothfactor = (smoothmin + 0.3 * (smoothmax - smoothmin))
						/ smoothweight;				
			smoothweight_old = smoothweight;
			}
		    else if (sigma_crossing > 1.01 * sigma_crossing_first
			    && sigma_crossing > 0.0)
			{
			if (smoothweight < smoothmax || smoothmax < 0.0)
			    {
			    smoothmax = smoothweight;
			    }
			smoothfactor = (smoothmin + 0.3 * (smoothmax - smoothmin))
					    / smoothweight;				
			smoothweight_old = smoothweight;
			}
		    else
			{
			smoothweight_old = smoothweight;
			}
		
		    /* do message */
		    sprintf(message, " >   %d %12g %12g %12g %12g\n",
			    iter, smoothweight, sigma_total, 
			    sigma_crossing, 
			    (sigma_crossing / sigma_crossing_first));
		    do_info_add(message, MB_NO);
fprintf(stderr,"iteration:%3d smooth:%12g sigmatot:%12g sigmacrossing:%12g ratio:%12g\n",
iter,smoothweight,sigma_total, 
sigma_crossing,
(sigma_crossing / sigma_crossing_first));
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
			    if (isnav == 0 && section->continuity == MB_YES)
				k -= 3;
			    section->snav_lon_offset[isnav] = (x[k] + file->block_offset_x) * mbna_mtodeglon;
			    section->snav_lat_offset[isnav] = (x[k+1] + file->block_offset_y) * mbna_mtodeglat;
			    section->snav_z_offset[isnav] = (x[k+2] + file->block_offset_z);
			    k += 3;
			    }
			}
		    }

		/* interpolate the solution */
		mbnavadjust_interpolatesolution();
		}
		
	/* output results from navigation solution */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed == project.num_crossings
			|| project.num_truecrossings_analyzed == project.num_truecrossings)
		&& error == MB_ERROR_NO_ERROR)
    		{
		
		/* now output inverse solution */
		sprintf(message,"Outputting navigation solution...");
		do_message_on(message);

		sprintf(message, " > Final smoothing weight:%12g\n > Final crossing sigma:%12g\n > Final total sigma:%12g\n",
			smoothweight_best, sigma_crossing_best, sigma_total_best);
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

		if (mbna_verbose > 0)
		for (i=0;i<nc/3;i++)
		    {
		    fprintf(stderr, "i:%d  offsets: %f %f %f  crossings: %d %d %d\n",
		    		i, x[3*i], x[3*i+1], x[3*i+2], nx[3*i], nx[3*i+1], nx[3*i+1]);
		    }
		if (mbna_verbose > 0)
		for (i=0;i<nr;i++)
		    {
		    s = 0.0;
		    for (j=0;j<nia[i];j++)
			{
			k = nnz * i + j;
			s += x[ia[k]] * a[k];
			if (mbna_verbose > 0)
			fprintf(stderr, "i:%4d j:%4d k:%4d ia[k]:%4d a[k]:%12g\n", i, j, k, ia[k], a[k]);
			}
		    if (mbna_verbose > 0)
		    fprintf(stderr, "i:%5d n:%5d  d:%12g s:%12g err:%12g\n",i,nia[i],d[i],s,d[i]-s);
		    }
		
		/* write updated project */
		project.inversion = MBNA_INVERSION_CURRENT;
		mbnavadjust_write_project();

		/* deallocate arrays */
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&a,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ia,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nia,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&d,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&w,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nx,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&dx,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&sigma,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&work,&error);

		/* generate new nav files */
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    if (file->status != MBNA_FILE_FIXED)
		    	{
		    	sprintf(npath,"%s/nvs_%4.4d.mb166",
			    project.datadir,i);
		    	sprintf(apath,"%s/nvs_%4.4d.na%d",
			    project.datadir,i,file->output_id);
		    	sprintf(opath,"%s.na%d", file->path, file->output_id);
		    	if ((nfp = fopen(npath,"r")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		    	else if ((afp = fopen(apath,"w")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		   	else if ((ofp = fopen(opath,"w")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		    	else
			    {
			    sprintf(message, " > Output updated nav to %s\n", opath);
			    do_info_add(message, MB_NO);

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
			    	else if ((nscan = sscanf(buffer, "%d %d %d %d %d %d.%d %lf %lf %lf %lf %lf %lf %lf %lf %lf",
							&time_i[0], &time_i[1], &time_i[2], &time_i[3],
							&time_i[4], &time_i[5], &time_i[6], &time_d,
							&navlon, &navlat, &heading, &speed, 
							&draft, &roll, &pitch, &heave)) >= 12)
				    {
				    /* fix nav from early version */
				    if (nscan < 16)
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
					navlon -= section->snav_lon_offset[isnav]
							+ factor * (section->snav_lon_offset[isnav+1]
								- section->snav_lon_offset[isnav]);
					navlat -= section->snav_lat_offset[isnav]
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
					sprintf(ostring, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						    time_i[0], time_i[1], time_i[2], time_i[3],
						    time_i[4], time_i[5], time_i[6], time_d,
						    navlon, navlat, heading, speed, 
						    draft, roll, pitch, heave, zoffset);
					sprintf(ostring, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						    time_i[0], time_i[1], time_i[2], time_i[3],
						    time_i[4], time_i[5], time_i[6], time_d,
						    navlon, navlat, heading, speed, 
						    draft, roll, pitch, heave, zoffset);
					fprintf(ofp, "%s", ostring);
					fprintf(afp, "%s", ostring);
/*fprintf(stderr, "%2.2d:%2.2d:%2.2d:%5.3f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f\n\n",
i, isection, isnav, factor, 
time_i[0], time_i[1], time_i[2], time_i[3],
time_i[4], time_i[5], time_i[6], time_d,
navlon, navlat, heading, speed, zoffset);*/
					}
				    }
			    	}
			    }
		    	if (nfp != NULL) fclose(nfp);
		    	if (afp != NULL) fclose(afp);
		    	if (ofp != NULL) 
			    {
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
mbnavadjust_invertnavold()
{
	/* local variables */
	char	*function_name = "mbnavadjust_invertnavold";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_file *file1;
	struct mbna_file *file2;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	nnav, nsnav, nfix, ndx, ndx2, ntie, nconstraint;
	int	nblock;
	int	nseq;
	int	nnz = 3;
	int	ndf = 3;
	int	nrows = 0;
	int	ncols = 0;
	double	*a;
	int	*ia;
	int	*nia;
	int	*nx;
	double	*x;
	double	*dx;
	double	*d;
	double	*sigma;
	double	*work;
	double	time_d, time_d_old, time_d_older, dtime_d;
	double	avg_dtime_d, avg_offset;
	int	ifile, icrossing, isection, isnav;
	int	nr, nc, nc1, nc2;
	int	ncyc, nsig;
	double	smax, sup, err, supt, slo, errlsq, s;
	int	ncycle = 2048;
	double	bandwidth = 10000.0;
	double	smoothweight, sigma_total, sigma_crossing;
	double	sigma_crossing_first;
	double	smoothweight_old, smoothweight_best, sigma_total_best, sigma_crossing_best;
	double	smoothfactor, smoothmin, smoothmax;
	double	offset_x, offset_y, offset_z;
	int	first;
	int	iter;
	char	npath[STRING_MAX];
	char	apath[STRING_MAX];
	char	opath[STRING_MAX];
	char	ostring[STRING_MAX];
	FILE	*nfp, *afp, *ofp;
	char	*result;
	char	buffer[BUFFER_MAX];
	int	done, nscan;
	double	factor;
	int	time_i[7];
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	double	zoffset;
	int	mbp_heading_mode;
	double	mbp_headingbias;
	int	mbp_rollbias_mode;
	double	mbp_rollbias;
	double	mbp_rollbias_port;
	double	mbp_rollbias_stbd;
	double	fixed_block_offset_x;
	double	fixed_block_offset_y;
	double	fixed_block_offset_z;
	int	nfixed_block_offset;
	int	i, j, k;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* invert if there is a project and all crossings have been analyzed */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed == project.num_crossings
			|| project.num_truecrossings_analyzed == project.num_truecrossings))
			
    		{
		/* set message dialog on */
		sprintf(message,"Setting up navigation inversion...");
		do_message_on(message);
		
		/*----------------------------------------------------------------*/
		
		/* figure out the average offsets between connected sets of files 
			- invert for x y z offsets for the blocks */
			
		/* count the number of blocks */
		nblock = 0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    if (i==0 || file->sections[0].continuity == MB_NO)
		    	{
			nblock++;
			}
		    file->block = nblock - 1;
		    file->block_offset_x = 0.0;
		    file->block_offset_y = 0.0;
		    file->block_offset_z = 0.0;
		    }
		ntie = 0;
		for (icrossing=0;icrossing<project.num_crossings;icrossing++)
		    {
		    crossing = &project.crossings[icrossing];
		    if (crossing->status == MBNA_CROSSING_STATUS_SET)
		    ntie += crossing->num_ties;
		    }
		
		/* invert for block offsets only if there is more than one block */
		if (nblock > 1)
		    {
		    /* allocate space for the inverse problem */
		    nconstraint = ndf * ntie;
		    nrows = nconstraint;
		    ncols = ndf * nblock;
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnz * nrows * sizeof(double), (void **)&a,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnz * nrows * sizeof(int), (void **)&ia,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows * sizeof(int), (void **)&nia,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows * sizeof(double), (void **)&d,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&x,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(int), (void **)&nx,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&dx,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncycle * sizeof(double), (void **)&sigma,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncycle * sizeof(double), (void **)&work,&error);

		    /* initialize arrays */
		    for (i=0;i<nrows;i++)
			{
			nia[i] = 0;
			d[i] = 0.0;
			for (j=0;j<nnz;j++)
			    {
			    k = nnz * i + j;
			    ia[k] = 0;
			    a[k] = 0.0;
			    }
			}
		    for (i=0;i<ndf*nblock;i++)
			{
			nx[i] = 0;
			x[i] = 0;
			dx[i] = 0.0;
			}
		    for (i=0;i<ncycle;i++)
			{
			sigma[i] = 0;
			work[i] = 0.0;
			}

		    /* construct the inverse problem */
		    sprintf(message,"Solving for block offsets...");
		    do_message_on(message);
		    
		    /* loop over crossings getting set ties */
		    nr = 0;
		    nc = ndf * nblock;
		    ntie = 0;
		    for (icrossing=0;icrossing<project.num_crossings;icrossing++)
			{
			crossing = &project.crossings[icrossing];

			/* use only set crossings */
			if (crossing->status == MBNA_CROSSING_STATUS_SET)
			for (j=0;j<crossing->num_ties;j++)
			    {
			    /* get tie */
			    tie = (struct mbna_tie *) &crossing->ties[j];
			    ntie++;

			    /* get block id for first snav point */
			    file = &project.files[crossing->file_id_1];
			    nc1 = file->block;

			    /* get block id for second snav point */
			    file = &project.files[crossing->file_id_2];
			    nc2 = file->block;

			    /* make longitude constraint */
			    k = nnz * nr;
			    a[k] = -1.0;
			    a[k+1] = 1.0;
			    d[nr] = tie->offset_x_m;
			    ia[k] = 3 * nc1;
			    ia[k+1] = 3 * nc2;
			    nia[nr] = 2;
			    nx[3 * nc1]++;
			    nx[3 * nc2]++;
/* fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%g\n", nr,k, a[k],a[k+1],d[nr]); */
			    nr++;

			    /* make latitude constraint */
			    k = nnz * nr;
			    a[k] = -1.0;
			    a[k+1] = 1.0;
			    d[nr] = tie->offset_y_m;
			    ia[k] = 3 * nc1 + 1;
			    ia[k+1] = 3 * nc2 + 1;
			    nia[nr] = 2;
			    nx[3 * nc1 + 1]++;
			    nx[3 * nc2 + 1]++;
/* fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%g\n", nr,k, a[k],a[k+1],d[nr]); */
			    nr++;

			    /* make depth constraint */
			    k = nnz * nr;
			    a[k] = -1.0;
			    a[k+1] = 1.0;
			    d[nr] = tie->offset_z_m;
			    ia[k] = 3 * nc1 + 2;
			    ia[k+1] = 3 * nc2 + 2;
			    nia[nr] = 2;
			    nx[3 * nc1 + 2]++;
			    nx[3 * nc2 + 2]++;
/* fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%g\n", nr,k, a[k],a[k+1],d[nr]); */
			    nr++;
			    }
			}

		    /* solve the inverse problem */

		    /* compute upper bound on maximum eigenvalue */
		    ncyc = 0;
		    nsig = 0;
		    lspeig(a, ia, nia, nnz, nc, nr, ncyc,
			    &nsig, x, dx, sigma, work, &smax, &err, &sup);
		    supt = smax + err;
		    if (sup > supt)
			supt = sup;
		    if (mbna_verbose > 0)
		    fprintf(stderr, "Initial lspeig: sup:%g smax:%g err:%g supt:%g\n",
			sup, smax, err, supt);
		    ncyc = 16;
		    for (i=0;i<4;i++)
			{
			lspeig(a, ia, nia, nnz, nc, nr, ncyc,
				&nsig, x, dx, sigma, work, &smax, &err, &sup);
			supt = smax + err;
			if (sup > supt)
			    supt = sup;
			if (mbna_verbose > 0)
			fprintf(stderr, "lspeig[%d]: sup:%g smax:%g err:%g supt:%g\n",
			    i, sup, smax, err, supt);
			}

		    /* calculate chebyshev factors (errlsq is the theoretical error) */
		    slo = supt / bandwidth;
		    chebyu(sigma, ncycle, supt, slo, work);
		    errlsq = errlim(sigma, ncycle, supt, slo);
		    if (mbna_verbose > 0)
		    fprintf(stderr, "Theoretical error: %f\n", errlsq);
		    if (mbna_verbose > 1)
		    for (i=0;i<ncycle;i++)
			fprintf(stderr, "sigma[%d]: %f\n", i, sigma[i]);

		    /* solve the problem */
		    for (i=0;i<nc;i++)
			x[i] = 0.0;
		    lsqup(a, ia, nia, nnz, nc, nr, x, dx, d, 0, NULL, NULL, ncycle, sigma);

		    /* output solution */
		    if (mbna_verbose > 1)
		    for (i=0;i<nc/3;i++)
			{
			fprintf(stderr, "block:%d  offsets: %f %f %f  nties: %d %d %d\n",
				i, x[3*i], x[3*i+1], x[3*i+2], nx[3*i], nx[3*i+1], nx[3*i+2]);
			}

		    /* calculate error */
		    sigma_total = 0.0;
		    sigma_crossing = 0.0;
		    for (i=0;i<nr;i++)
			{
			s = 0.0;
			for (j=0;j<nia[i];j++)
			    {
			    k = nnz * i + j;
			    s += x[ia[k]] * a[k];
			    }
			sigma_total += (d[i] - s) * (d[i] - s);
			if (i >= (nr - 3 * ntie))
			    sigma_crossing += (d[i] - s) * (d[i] - s);
			}
		    sigma_total = sqrt(sigma_total) / nr;
		    sigma_crossing = sqrt(sigma_crossing) / ntie;

		    /* extract results */
		    fixed_block_offset_x = 0.0;
		    fixed_block_offset_y = 0.0;
		    fixed_block_offset_z = 0.0;
		    nfixed_block_offset = 0;
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			if (file->status == MBNA_FILE_FIXED)
				{
				fixed_block_offset_x -= x[3 * file->block];
				fixed_block_offset_y -= x[3 * file->block + 1];
				fixed_block_offset_z -= x[3 * file->block + 2];
				nfixed_block_offset++;
				}
			file->block_offset_x = x[3 * file->block];
			file->block_offset_y = x[3 * file->block + 1];
			file->block_offset_z = x[3 * file->block + 2];
			}
		    if (nfixed_block_offset > 0)
		        {
			fixed_block_offset_x /= (double)nfixed_block_offset;
			fixed_block_offset_y /= (double)nfixed_block_offset;
			fixed_block_offset_z /= (double)nfixed_block_offset;
			for (i=0;i<project.num_files;i++)
			    {
			    file = &project.files[i];
			    file->block_offset_x += fixed_block_offset_x;
			    file->block_offset_y += fixed_block_offset_y;
			    file->block_offset_z += fixed_block_offset_z;
			    }
			}
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
fprintf(stderr,"file:%d block: %d block offsets: %f %f %f\n",
i,file->block,file->block_offset_x,file->block_offset_y,file->block_offset_z);
			}

		    /* deallocate arrays */
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&a,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ia,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nia,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&d,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nx,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&dx,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&sigma,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&work,&error);
		    }
		
		/*----------------------------------------------------------------*/		

     		/* retrieve crossing parameters */
		/* count up the unknowns and constraints to get size of
		 * inverse problem:
		 *	    nconstraint = ndf * nsnav + ntie
		 */
		nnav = 0;
		nsnav = 0;
		nfix = 0;
		ntie = 0;
		nseq = 0;
		ndx = 0;
		ndx2 = 0;
		avg_dtime_d = 0.0;
		for (i=0;i<project.num_files;i++)
		    {
sprintf(message,"Processing file %d of %d...", i, project.num_files);
do_message_on(message);
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			nnav += section->num_pings;
			nsnav += section->num_snav - section->continuity;
			if (file->status == MBNA_FILE_FIXED)
			    nfix += section->num_snav - section->continuity;
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    if (isnav > 0 || section->continuity == MB_NO)
				{
				/* handle sequence count */
				if (isnav > 0)
				    nseq++;
				else
				    nseq = 1;
				    
				/* get time difference between current and previous nav point */
				dtime_d = section->snav_time_d[isnav] - time_d_old;
				
				/* add first derivative constraint if nseq > 1 AND dtime_d > 0.0 */
				if (nseq > 1 && dtime_d > 0.0)
				    {
				    avg_dtime_d += dtime_d;
				    ndx++;
				    }
				
				/* add second derivative constraint if nseq > 2 */
				if (nseq > 2 && dtime_d > 0.0)
				    ndx2++;
				
				/* save time_d values */
				time_d_old = section->snav_time_d[isnav];
				}
			    }
			}
		    }
		if (ndx > 0)
		    avg_dtime_d /= ndx;
		avg_offset = 0.0;
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &project.crossings[i];
		    if (crossing->status == MBNA_CROSSING_STATUS_SET
		    	&& crossing->num_ties > 0)
			{
			ntie += crossing->num_ties;
			for (j=0;j<crossing->num_ties;j++)
			    {
			    avg_offset += fabs(crossing->ties[j].offset_x);
			    }
			}
		    }
		if (ntie > 0)
		    avg_offset /= ntie;
		
		/* allocate space for the inverse problem */
		nconstraint = ndf * (nfix + ndx + ndx2 + ntie);
		nrows = nconstraint;
		ncols = ndf * nsnav;
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnz * nrows * sizeof(double), (void **)&a,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnz * nrows * sizeof(int), (void **)&ia,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows * sizeof(int), (void **)&nia,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows * sizeof(double), (void **)&d,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&x,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(int), (void **)&nx,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&dx,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncycle * sizeof(double), (void **)&sigma,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncycle * sizeof(double), (void **)&work,&error);

		/* if error initializing memory then don't invert */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(mbna_verbose,error,&error_message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
				error_message);
			}
		}
		
	/* proceed with construction of inverse problems */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed == project.num_crossings
			|| project.num_truecrossings_analyzed == project.num_truecrossings)
		&& error == MB_ERROR_NO_ERROR)
    		{
		/* add info text */
		sprintf(message, "Inverting for optimal navigation\n");
		do_info_add(message, MB_YES);
		sprintf(message, " > Inverse problem size:\n");
		do_info_add(message, MB_NO);
		sprintf(message, " >   Nav points:                    %d\n", nnav);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Independent nav snav points:   %d\n", nsnav);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Fixed nav snav points:         %d\n", nfix);
		do_info_add(message, MB_NO);
		sprintf(message, " >   First derivative constraints:  %d\n", ndx);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Second derivative constraints: %d\n", ndx2);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Nav ties:                      %d\n", ntie);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Total Unknowns:                %d\n", 3 * nsnav);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Total Constraints:             %d\n", nconstraint);
		do_info_add(message, MB_NO);
		sprintf(message, " > Iteration Smoothing S_total S_crossing Ratio\n > --------------------------------------\n");
		do_info_add(message, MB_NO);
		
		/* initialize arrays */
		for (i=0;i<nrows;i++)
		    {
		    nia[i] = 0;
		    d[i] = 0.0;
		    for (j=0;j<nnz;j++)
			{
			k = nnz * i + j;
			ia[k] = 0;
			a[k] = 0.0;
			}
		    }
		for (i=0;i<ndf*nsnav;i++)
		    {
		    nx[i] = 0;
		    x[i] = 0;
		    dx[i] = 0.0;
		    }
		for (i=0;i<ncycle;i++)
		    {
		    sigma[i] = 0;
		    work[i] = 0.0;
		    }
		
		/* make inverse problem */
		/*------------------------*/
		nr = 0;
		smoothweight = avg_dtime_d * avg_offset / 100000000;
		smoothweight_old = smoothweight;
		
		/* apply first and second derivative constraints */
		nc = 0;
		nseq = 0;
		for (ifile=0;ifile<project.num_files;ifile++)
		    {
sprintf(message,"Applying derivatives to file %d of %d...", ifile, project.num_files);
do_message_on(message);
		    file = &project.files[ifile];
		    for (isection=0;isection<file->num_sections;isection++)
			{
			section = &file->sections[isection];
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {			
			    /* if continuity true first snav point is same as the last
			       snav point of previous section */
			    if (isnav > 0 || section->continuity == MB_NO)
				{
				/* handle sequence count */
				if (isnav > 0)
				    nseq++;
				else
				    nseq = 1;
				    
			    	/* if file fixed then fix snav point */
			    	if (file->status == MBNA_FILE_FIXED)
				    {
				    /* longitude component */
				    k = nnz * nr;
				    a[k] = 1.0;
				    d[nr] = 0.0;
				    ia[k] = nc;
				    nia[nr] = 1;
				    nr++;
				
				    /* latitude component */
				    k = nnz * nr;
				    a[k] = 1.0;
				    d[nr] = 0.0;
				    ia[k] = nc + 1;
				    nia[nr] = 1;
				    nr++;
				
				    /* depth component */
				    k = nnz * nr;
				    a[k] = 100.0;
				    d[nr] = 0.0;
				    ia[k] = nc + 2;
				    nia[nr] = 1;
				    nr++;
				    }
				    
				/* get time difference between current and previous nav point */
				dtime_d = section->snav_time_d[isnav] - time_d_old;
				
				/* add first derivative constraint if nseq > 1 AND dtime_d > 0.0 */
				if (nseq > 1 && dtime_d > 0.0)
				    {
				    /* longitude component */
				    k = nnz * nr;
				    a[k] = -smoothweight_old / dtime_d;
				    a[k+1] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 3;
				    ia[k+1] = nc;
				    nia[nr] = 2;
				    nr++;
				
				    /* latitude component */
				    k = nnz * nr;
				    a[k] = -smoothweight_old / dtime_d;
				    a[k+1] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 2;
				    ia[k+1] = nc + 1;
				    nia[nr] = 2;
				    nr++;
				
				    /* depth component */
				    k = nnz * nr;
				    a[k] = -smoothweight_old / dtime_d;
				    a[k+1] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 1;
				    ia[k+1] = nc + 2;
				    nia[nr] = 2;
				    nr++;
				    }
				
				/* add second derivative constraint if nseq > 2  AND dtime_d > 0.0 */
				if (nseq > 2 && dtime_d > 0.0)
				    {
				    /* longitude component */
				    k = nnz * nr;
				    a[k] = smoothweight_old / dtime_d;
				    a[k+1] = -2.0 * smoothweight_old / dtime_d;
				    a[k+2] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 6;
				    ia[k+1] = nc - 3;
				    ia[k+2] = nc;
				    nia[nr] = 3;
				    nr++;
				
				    /* latitude component */
				    k = nnz * nr;
				    a[k] = smoothweight_old / dtime_d;
				    a[k+1] = -2.0 * smoothweight_old / dtime_d;
				    a[k+2] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 5;
				    ia[k+1] = nc - 2;
				    ia[k+2] = nc + 1;
				    nia[nr] = 3;
				    nr++;
				
				    /* depth component */
				    k = nnz * nr;
				    a[k] = smoothweight_old / dtime_d;
				    a[k+1] = -2.0 * smoothweight_old / dtime_d;
				    a[k+2] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 4;
				    ia[k+1] = nc - 1;
				    ia[k+2] = nc + 2;
				    nia[nr] = 3;
				    nr++;
				    }
				
				/* save time_d values */
				time_d_older = time_d_old;
				time_d_old = section->snav_time_d[isnav];

				/* add to unknown count */
				nc += ndf;
				}
			    }
			}
		    }
		
		/* apply crossing offset constraints */
sprintf(message,"Applying %d crossing constraints...", project.num_crossings);
do_message_on(message);
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
			section = &file1->sections[crossing->section_1];
			nc1 = section->global_start_snav + tie->snav_1;

			/* get absolute id for second snav point */
			file2 = &project.files[crossing->file_id_2];
			section = &file2->sections[crossing->section_2];
			nc2 = section->global_start_snav + tie->snav_2;
if (nc1 > nsnav - 1 || nc2 > nsnav -1
|| nc1 < 0 || nc2 < 0)
fprintf(stderr, "BAD snav ID: %d %d %d\n", nc1, nc2, nsnav);
			
			/* make longitude constraint */
			k = nnz * nr;
			a[k] = -mbna_offsetweight;
			a[k+1] = mbna_offsetweight;
			d[nr] = mbna_offsetweight * (tie->offset_x_m - file2->block_offset_x + file1->block_offset_x);
			ia[k] = 3 * nc1;
			ia[k+1] = 3 * nc2;
			nia[nr] = 2;
			nx[3 * nc1]++;
			nx[3 * nc2]++;
/*fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%d\n", nr,k, a[k],a[k+1],d[nr]);*/
			nr++;
			
			/* make latitude constraint */
			k = nnz * nr;
			a[k] = -mbna_offsetweight;
			a[k+1] = mbna_offsetweight;
			d[nr] = mbna_offsetweight * (tie->offset_y_m  - file2->block_offset_y + file1->block_offset_y);
			ia[k] = 3 * nc1 + 1;
			ia[k+1] = 3 * nc2 + 1;
			nia[nr] = 2;
			nx[3 * nc1 + 1]++;
			nx[3 * nc2 + 1]++;
/*fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%d\n", nr,k, a[k],a[k+1],d[nr]);*/
			nr++;
			
			/* make depth constraint */
			k = nnz * nr;
			a[k] = -mbna_offsetweight;
			a[k+1] = mbna_offsetweight;
			d[nr] = mbna_offsetweight * (tie->offset_z_m - file2->block_offset_z + file1->block_offset_z);
			ia[k] = 3 * nc1 + 2;
			ia[k+1] = 3 * nc2 + 2;
			nia[nr] = 2;
			nx[3 * nc1 + 2]++;
			nx[3 * nc2 + 2]++;
/*fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%d\n", nr,k, a[k],a[k+1],d[nr]);*/
			nr++;
			}
		    }
		    
/* print out matrix problem & calculate error */
/*sigma_total = 0.0;
sigma_crossing = 0.0;
fprintf(stderr,"START PROBLEM: smoothweight:%g avg_dtime_d:%g avg_offset:%g\n",
smoothweight,avg_dtime_d,avg_offset);

for (i=0;i<nr;i++)
{
fprintf(stderr,"row %d with %d elements\n", i, nia[i]);
s = 0.0;
for (j=0;j<nia[i];j++)
    {
    k = nnz * i + j;
    s += x[ia[k]] * a[k];
fprintf(stderr,"element %d column %d    a:%g  x:%g  s:%g\n", j, ia[k], a[k], x[ia[k]], x[ia[k]] * a[k]);
    }
sigma_total += (d[i] - s) * (d[i] - s);
fprintf(stderr,"row %d result:  s:%g d:%g sigma_total:%g %g\n\n", i, s, d[i], (d[i] - s) * (d[i] - s), sigma_total);
if (i >= (nr - ndf * ntie))
    sigma_crossing += (d[i] - s) * (d[i] - s);
}
sigma_total = sqrt(sigma_total) / nr;
sigma_crossing = sqrt(sigma_crossing) / ntie;
fprintf(stderr,"INITIAL SIGMA: sigma_total:%g sigma_crossing:%g\n", sigma_total, sigma_crossing);
*/

		/* now do a bunch of test solutions to find what level of smoothing
		 * starts to impact the solution - we want to use this much and no more
		 */
		first = MB_YES;
		done = MB_NO;
		iter=0;
		smoothfactor = 100.0;
		smoothmin = -1.0;
		smoothmax = -1.0;
		while (done == MB_NO)
		    {
		    /* first change the smoothing weight in the matrix problem */
		    smoothweight = smoothweight * smoothfactor;
		    iter++;
		    for (i=0;i<ndf*(nfix+ndx+ndx2);i++)
			{
			for (j=0;j<nia[i];j++)
			    {
			    if (nia[i] > 1)
				{
				k = nnz * i + j;
				a[k] *= smoothfactor;
				}
			    }
			}
		
		    /* now solve inverse problem */
		    if (first == MB_YES)
			{
			sprintf(message,"Inverting %dx%d: iter:%d smooth:%.2g",
				nc, nr, iter, smoothweight);
			}
		    else
			{
			sprintf(message,"Inverting %dx%d: iter:%d smooth:%.2g ratio:%.3f iter:%d smooth:%.2g ",
				nc, nr, iter-1, smoothweight_old,
				(sigma_crossing / sigma_crossing_first), iter, smoothweight);
			}
		    do_message_on(message);
		
		    /* compute upper bound on maximum eigenvalue */
		    ncyc = 0;
		    nsig = 0;
		    lspeig(a, ia, nia, nnz, nc, nr, ncyc,
			    &nsig, x, dx, sigma, work, &smax, &err, &sup);
		    supt = smax + err;
		    if (sup > supt)
			supt = sup;
		    if (mbna_verbose > 0)
		    fprintf(stderr, "Initial lspeig: sup:%g smax:%g err:%g supt:%g\n",
			sup, smax, err, supt);
		    ncyc = 16;
		    for (i=0;i<4;i++)
			{
			lspeig(a, ia, nia, nnz, nc, nr, ncyc,
				&nsig, x, dx, sigma, work, &smax, &err, &sup);
			supt = smax + err;
			if (sup > supt)
			    supt = sup;
			if (mbna_verbose > 0)
			fprintf(stderr, "lspeig[%d]: sup:%g smax:%g err:%g supt:%g\n",
			    i, sup, smax, err, supt);
			}
			
		    /* calculate chebyshev factors (errlsq is the theoretical error) */
		    slo = supt / bandwidth;
		    chebyu(sigma, ncycle, supt, slo, work);
		    errlsq = errlim(sigma, ncycle, supt, slo);
		    if (mbna_verbose > 0)
		    fprintf(stderr, "Theoretical error: %f\n", errlsq);
		    if (mbna_verbose > 1)
		    for (i=0;i<ncycle;i++)
			fprintf(stderr, "sigma[%d]: %f\n", i, sigma[i]);
			
		    /* solve the problem */
		    for (i=0;i<nc;i++)
			x[i] = 0.0;
		    lsqup(a, ia, nia, nnz, nc, nr, x, dx, d, 0, NULL, NULL, ncycle, sigma);
	
		    /* output solution */
		    if (mbna_verbose > 1)
		    for (i=0;i<nc/3;i++)
			{
			fprintf(stderr, "i:%d  offsets: %f %f %f  crossings: %d %d %d\n",
				i, x[3*i], x[3*i+1], x[3*i+2], nx[3*i], nx[3*i+1], nx[3*i+2]);
			}
		
		    /* calculate error */
		    sigma_total = 0.0;
		    sigma_crossing = 0.0;
		    for (i=0;i<nr;i++)
			{
			s = 0.0;
			for (j=0;j<nia[i];j++)
			    {
			    k = nnz * i + j;
			    s += x[ia[k]] * a[k];
			    }
			sigma_total += (d[i] - s) * (d[i] - s);
			if (i >= (nr - ndf * ntie))
			    sigma_crossing += (d[i] - s) * (d[i] - s);
			}
		    sigma_total = sqrt(sigma_total) / nr;
		    sigma_crossing = sqrt(sigma_crossing) / ntie;
		
		    /* keep track of results */
		    if (first == MB_YES)
			{
			first = MB_NO;
			sigma_crossing_first = MAX(sigma_crossing, project.precision);
			smoothweight_old = smoothweight;
			smoothmin = smoothweight;
			}
		    else if (sigma_crossing >= 1.005 * sigma_crossing_first
			    && sigma_crossing <= 1.01 * sigma_crossing_first
			    && sigma_crossing > 0.0)
			{
			done = MB_YES;
			smoothweight_best = smoothweight;
			sigma_total_best = sigma_total;
			sigma_crossing_best = sigma_crossing;
			smoothweight_old = smoothweight;
			}
		    else if (sigma_crossing < 1.005 * sigma_crossing_first)
			{
			if (smoothweight > smoothmin)
			    {
			    smoothmin = smoothweight;
			    }
			if (smoothmax > 0.0)
			    smoothfactor = (smoothmin + 0.3 * (smoothmax - smoothmin))
						/ smoothweight;				
			smoothweight_old = smoothweight;
			}
		    else if (sigma_crossing > 1.01 * sigma_crossing_first
			    && sigma_crossing > 0.0)
			{
			if (smoothweight < smoothmax || smoothmax < 0.0)
			    {
			    smoothmax = smoothweight;
			    }
			smoothfactor = (smoothmin + 0.3 * (smoothmax - smoothmin))
					    / smoothweight;				
			smoothweight_old = smoothweight;
			}
		    else
			{
			smoothweight_old = smoothweight;
			}
		
		    /* do message */
		    sprintf(message, " >   %d %12g %12g %12g %12g\n",
			    iter, smoothweight, sigma_total, 
			    sigma_crossing, 
			    (sigma_crossing / sigma_crossing_first));
		    do_info_add(message, MB_NO);
/* fprintf(stderr,"iteration:%3d smooth:%12g sigmatot:%12g sigmacrossing:%12g ratio:%12g\n",
iter,smoothweight,sigma_total, 
sigma_crossing,
(sigma_crossing / sigma_crossing_first));*/
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
			    if (isnav == 0 && section->continuity == MB_YES)
				k -= 3;
			    section->snav_lon_offset[isnav] = (x[k] + file->block_offset_x) * mbna_mtodeglon;
			    section->snav_lat_offset[isnav] = (x[k+1] + file->block_offset_y) * mbna_mtodeglat;
			    section->snav_z_offset[isnav] = (x[k+2] + file->block_offset_z);
			    k += 3;
			    }
			}
		    }

		/* interpolate the solution */
		mbnavadjust_interpolatesolution();
		}
		
	/* output results from navigation solution */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed == project.num_crossings
			|| project.num_truecrossings_analyzed == project.num_truecrossings)
		&& error == MB_ERROR_NO_ERROR)
    		{
		
		/* now output inverse solution */
		sprintf(message,"Outputting navigation solution...");
		do_message_on(message);

		sprintf(message, " > Final smoothing weight:%12g\n > Final crossing sigma:%12g\n > Final total sigma:%12g\n",
			smoothweight_best, sigma_crossing_best, sigma_total_best);
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

		if (mbna_verbose > 0)
		for (i=0;i<nc/3;i++)
		    {
		    fprintf(stderr, "i:%d  offsets: %f %f %f  crossings: %d %d %d\n",
		    		i, x[3*i], x[3*i+1], x[3*i+2], nx[3*i], nx[3*i+1], nx[3*i+1]);
		    }
		if (mbna_verbose > 0)
		for (i=0;i<nr;i++)
		    {
		    s = 0.0;
		    for (j=0;j<nia[i];j++)
			{
			k = nnz * i + j;
			s += x[ia[k]] * a[k];
			if (mbna_verbose > 0)
			fprintf(stderr, "i:%4d j:%4d k:%4d ia[k]:%4d a[k]:%12g\n", i, j, k, ia[k], a[k]);
			}
		    if (mbna_verbose > 0)
		    fprintf(stderr, "i:%5d n:%5d  d:%12g s:%12g err:%12g\n",i,nia[i],d[i],s,d[i]-s);
		    }
		
		/* write updated project */
		project.inversion = MBNA_INVERSION_CURRENT;
		mbnavadjust_write_project();

		/* deallocate arrays */
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&a,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ia,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nia,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&d,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nx,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&dx,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&sigma,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&work,&error);

		/* generate new nav files */
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    if (file->status != MBNA_FILE_FIXED)
		    	{
		    	sprintf(npath,"%s/nvs_%4.4d.mb166",
			    project.datadir,i);
		    	sprintf(apath,"%s/nvs_%4.4d.na%d",
			    project.datadir,i,file->output_id);
		    	sprintf(opath,"%s.na%d", file->path, file->output_id);
		    	if ((nfp = fopen(npath,"r")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		    	else if ((afp = fopen(apath,"w")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		   	else if ((ofp = fopen(opath,"w")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		    	else
			    {
			    sprintf(message, " > Output updated nav to %s\n", opath);
			    do_info_add(message, MB_NO);

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
			    	else if ((nscan = sscanf(buffer, "%d %d %d %d %d %d.%d %lf %lf %lf %lf %lf %lf %lf %lf %lf",
							&time_i[0], &time_i[1], &time_i[2], &time_i[3],
							&time_i[4], &time_i[5], &time_i[6], &time_d,
							&navlon, &navlat, &heading, &speed, 
							&draft, &roll, &pitch, &heave)) >= 12)
				    {
				    /* fix nav from early version */
				    if (nscan < 16)
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
					navlon -= section->snav_lon_offset[isnav]
							+ factor * (section->snav_lon_offset[isnav+1]
								- section->snav_lon_offset[isnav]);
					navlat -= section->snav_lat_offset[isnav]
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
					sprintf(ostring, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						    time_i[0], time_i[1], time_i[2], time_i[3],
						    time_i[4], time_i[5], time_i[6], time_d,
						    navlon, navlat, heading, speed, 
						    draft, roll, pitch, heave, zoffset);
					sprintf(ostring, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						    time_i[0], time_i[1], time_i[2], time_i[3],
						    time_i[4], time_i[5], time_i[6], time_d,
						    navlon, navlat, heading, speed, 
						    draft, roll, pitch, heave, zoffset);
					fprintf(ofp, "%s", ostring);
					fprintf(afp, "%s", ostring);
/*fprintf(stderr, "%2.2d:%2.2d:%2.2d:%5.3f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f\n\n",
i, isection, isnav, factor, 
time_i[0], time_i[1], time_i[2], time_i[3],
time_i[4], time_i[5], time_i[6], time_d,
navlon, navlat, heading, speed, zoffset);*/
					}
				    }
			    	}
			    }
		    	if (nfp != NULL) fclose(nfp);
		    	if (afp != NULL) fclose(afp);
		    	if (ofp != NULL) 
			    {
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
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
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
		    if (section->snav_num_ties[isnav] > 0)
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
							factor = (psection->snav_time_d[iisnav] - ptime_d) 
								/ (section->snav_time_d[isnav] - ptime_d);
						else
							factor = 0.5;
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

int
mbnavadjust_invertnav2()
{
	/* local variables */
	char	*function_name = "mbnavadjust_invertnav2";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	nnav, nsnav, nsnavuse, nfix, ndx, ndx2, ntie, nconstraint;
	int	nseq;
	int	nnz = 3;
	int	ndf = 3;
	int	nrows = 0;
	int	ncols = 0;
	double	*a;
	int	*ia;
	int	*nia;
	int	*nx;
	double	*x;
	double	*dx;
	double	*d;
	double	*sigma;
	double	*work;
	double	*xtime_d;
	int	*xcontinuity;
	double	time_d, time_d_old, dtime_d;
	double	avg_dtime_d, avg_offset;
	int	ifile, icrossing, isection, isnav;
	int	nr, nc, nc1, nc2;
	int	ncyc, nsig;
	double	smax, sup, err, supt, slo, errlsq, s;
	int	ncycle = 2048;
	double	bandwidth = 10000.0;
	double	smoothweight, sigma_total, sigma_crossing;
	double	sigma_crossing_first;
	double	smoothweight_old, smoothweight_best, sigma_total_best, sigma_crossing_best;
	double	smoothfactor, smoothmin, smoothmax;
	double	offset_x, offset_y, offset_z;
	int	first, last;
	int	iter;
	char	npath[STRING_MAX];
	char	apath[STRING_MAX];
	char	opath[STRING_MAX];
	FILE	*nfp, *afp, *ofp;
	char	*result;
	char	buffer[BUFFER_MAX];
	int	done, nscan;
	double	factor;
	int	time_i[7];
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	double	zoffset;
	int	mbp_heading_mode;
	double	mbp_headingbias;
	int	mbp_rollbias_mode;
	double	mbp_rollbias;
	double	mbp_rollbias_port;
	double	mbp_rollbias_stbd;
	double	interp_value_lon, interp_value_lat, interp_value_z;
	int	interp_mode;
	int	i0, i1, ii, ic;
	int	i, j, k;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* invert if there is a project and all crossings have been analyzed */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed == project.num_crossings
			|| project.num_truecrossings_analyzed == project.num_truecrossings))
    		{
     		/* retrieve crossing parameters */
		/* count up the unknowns and constraints to get size of
		 * inverse problem:
		 *	    nconstraint = ndf * (nsnav + ntie)
		 */
		/* set message dialog on */
		sprintf(message,"Setting up navigation inversion...");
		do_message_on(message);

		/* count constraints */
		nnav = 0;
		nsnav = 0;
		nsnavuse = 0;
		nfix = 0;
		ntie = 0;
		nseq = 0;
		ndx = 0;
		ndx2 = 0;
		avg_dtime_d = 0.0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			nnav += section->num_pings;
			nsnav += section->num_snav - section->continuity;
			if (section->continuity == MB_NO)
			    nseq = 0;
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    if (section->snav_num_ties[isnav] > 0)
				{
			    	nseq++;
				nsnavuse++;
				if (nseq > 1)
				    {
				    dtime_d = section->snav_time_d[isnav] - time_d_old;
				    avg_dtime_d += dtime_d;
				    ndx++;
				    }
				if (nseq > 2)
				    ndx2++;
				if (file->status == MBNA_FILE_FIXED)
				    nfix ++;
				
				/* save time_d values */
				time_d_old = section->snav_time_d[isnav];
				}
			    if (nseq > 0)
			    	section->snav_invert_constraint[isnav] = 1;
			    else
			    	section->snav_invert_constraint[isnav] = 0;
			    }
			}
		    }
		nseq = 0;
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
			    	nseq++;
				}
			    if (nseq > 0)
			    	section->snav_invert_constraint[isnav] += 2;
			    }
			if (section->continuity == MB_NO)
			    nseq = 0;
			}
		    }
		if (ndx > 0)
		    avg_dtime_d /= ndx;
		avg_offset = 0.0;
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &project.crossings[i];
		    if (crossing->status == MBNA_CROSSING_STATUS_SET
		    	&& crossing->num_ties > 0)
			{
			ntie += crossing->num_ties;
			for (j=0;j<crossing->num_ties;j++)
			    {
			    avg_offset += fabs(crossing->ties[j].offset_x);
			    }
			}
		    }
		if (ntie > 0)
		    avg_offset /= ntie;
		
		/* allocate space for the inverse problem */
		nconstraint = ndf * (nfix + ndx + ndx2 + ntie);
		nrows = nconstraint;
		ncols = ndf * nsnavuse;
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnz * nrows * sizeof(double), (void **)&a,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnz * nrows * sizeof(int), (void **)&ia,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows * sizeof(int), (void **)&nia,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows * sizeof(double), (void **)&d,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&x,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(int), (void **)&nx,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&dx,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncycle * sizeof(double), (void **)&sigma,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncycle * sizeof(double), (void **)&work,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nsnavuse * sizeof(double), (void **)&xtime_d,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nsnavuse * sizeof(int), (void **)&xcontinuity,&error);

		/* if error initializing memory then don't invert */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(mbna_verbose,error,&error_message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
				error_message);
			}
		}
		
	/* proceed with construction of inverse problems */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed == project.num_crossings
			|| project.num_truecrossings_analyzed == project.num_truecrossings)
		&& error == MB_ERROR_NO_ERROR)
    		{
		/* add info text */
		sprintf(message, "Inverting for optimal navigation\n");
		do_info_add(message, MB_YES);
		sprintf(message, " > Inverse problem size:\n");
		do_info_add(message, MB_NO);
		sprintf(message, " >   Nav points:                    %d\n", nnav);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Independent nav snav points:   %d\n", nsnav);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Used nav snav points:          %d\n", nsnavuse);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Fixed nav snav points:         %d\n", nfix);
		do_info_add(message, MB_NO);
		sprintf(message, " >   First derivative constraints:  %d\n", ndx);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Second derivative constraints: %d\n", ndx2);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Useful ties:                   %d\n", ntie);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Total Unknowns:                %d\n", 3 * nsnavuse);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Total Constraints:             %d\n", nconstraint);
		do_info_add(message, MB_NO);
		sprintf(message, " > Iteration Smoothing S_total S_crossing Ratio\n > --------------------------------------\n");
		do_info_add(message, MB_NO);
		
		/* initialize arrays */
		for (i=0;i<nrows;i++)
		    {
		    nia[i] = 0;
		    d[i] = 0.0;
		    for (j=0;j<nnz;j++)
			{
			k = nnz * i + j;
			ia[k] = 0;
			a[k] = 0.0;
			}
		    }
		for (i=0;i<ncols;i++)
		    {
		    nx[i] = 0;
		    x[i] = 0;
		    dx[i] = 0.0;
		    }
		for (i=0;i<ncycle;i++)
		    {
		    sigma[i] = 0;
		    work[i] = 0.0;
		    }
		
		/* make inverse problem */
		/*------------------------*/
		nr = 0;
		smoothweight = avg_dtime_d * avg_offset / 100000000;
		smoothweight_old = smoothweight;
		
		/* apply first and second derivative constraints */
		nc = 0;
		nseq = 0;
		for (ifile=0;ifile<project.num_files;ifile++)
		    {
		    file = &project.files[ifile];
		    for (isection=0;isection<file->num_sections;isection++)
			{
			section = &file->sections[isection];
			if (section->continuity == MB_NO)
			    nseq = 0;
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {			
			    if (section->snav_num_ties[isnav] > 0)
				{
			    	nseq++;
				
				/* save time and continuity */
				xtime_d[nc/3] = section->snav_time_d[isnav];
				section->snav_invert_id[isnav] = nc / 3;
				if (nseq > 1)
				    xcontinuity[nc/3] = MB_YES;
				else
				    xcontinuity[nc/3] = MB_NO;

				/* add first derivative constraint */
				if (nseq > 1)
				    {
				    dtime_d = xtime_d[nc/3] - xtime_d[(nc-1)/3];
				    avg_dtime_d += dtime_d;

				    /* longitude component */
				    k = nnz * nr;
				    a[k] = -smoothweight_old / dtime_d;
				    a[k+1] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 3;
				    ia[k+1] = nc;
				    nia[nr] = 2;
				    nr++;
				
				    /* latitude component */
				    k = nnz * nr;
				    a[k] = -smoothweight_old / dtime_d;
				    a[k+1] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 2;
				    ia[k+1] = nc + 1;
				    nia[nr] = 2;
				    nr++;
				
				    /* depth offset component */
				    k = nnz * nr;
				    a[k] = -smoothweight_old / dtime_d;
				    a[k+1] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 1;
				    ia[k+1] = nc + 2;
				    nia[nr] = 2;
				    nr++;
				    }
				
				/* add second derivative constraint */
				if (nseq > 2)
				    {
				    dtime_d = xtime_d[nc/3] - xtime_d[(nc-2)/3];

				    /* longitude component */
				    k = nnz * nr;
				    a[k] = 10.0 * smoothweight_old / dtime_d;
				    a[k+1] = -2.0 * 10.0 * smoothweight_old / dtime_d;
				    a[k+2] = 10.0 * smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 6;
				    ia[k+1] = nc - 3;
				    ia[k+2] = nc;
				    nia[nr] = 3;
				    nr++;
				
				    /* latitude component */
				    k = nnz * nr;
				    a[k] = 10.0 * smoothweight_old / dtime_d;
				    a[k+1] = -2.0 * 10.0 * smoothweight_old / dtime_d;
				    a[k+2] = 10.0 * smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 5;
				    ia[k+1] = nc - 2;
				    ia[k+2] = nc + 1;
				    nia[nr] = 3;
				    nr++;
				
				    /* depth offset component */
				    k = nnz * nr;
				    a[k] = 10.0 * smoothweight_old / dtime_d;
				    a[k+1] = -2.0 * 10.0 * smoothweight_old / dtime_d;
				    a[k+2] = 10.0 * smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 4;
				    ia[k+1] = nc - 1;
				    ia[k+2] = nc + 2;
				    nia[nr] = 3;
				    nr++;
				    }
				    
				/* add fixed point constraint */
				if (file->status == MBNA_FILE_FIXED)
				    {
				    /* longitude component */
				    k = nnz * nr;
				    a[k] = 1.0;
				    d[nr] = 0.0;
				    ia[k] = nc;
				    nia[nr] = 1;
				    nr++;
				
				    /* latitude component */
				    k = nnz * nr;
				    a[k] = 1.0;
				    d[nr] = 0.0;
				    ia[k] = nc + 1;
				    nia[nr] = 1;
				    nr++;
				
				    /* depth offset component */
				    k = nnz * nr;
				    a[k] = 1.0;
				    d[nr] = 0.0;
				    ia[k] = nc + 2;
				    nia[nr] = 1;
				    nr++;
				    }
				
				/* increment columns */
				nc += 3;
				}
				
			    else
				section->snav_invert_id[isnav] = -1;
			    }
			}
		    }
		
		/* apply crossing offset constraints */
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
			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];
			nc1 = section->snav_invert_id[tie->snav_1];

			/* get absolute id for second snav point */
			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];
			nc2 = section->snav_invert_id[tie->snav_2];
if (nc1 > nsnav - 1 || nc2 > nsnav -1
|| nc1 < 0 || nc2 < 0)
fprintf(stderr, "BAD snav ID: %d %d %d\n", nc1, nc2, nsnav);
			
			/* make longitude constraint */
			k = nnz * nr;
			a[k] = -mbna_offsetweight;
			a[k+1] = mbna_offsetweight;
			d[nr] = mbna_offsetweight * tie->offset_x;
			ia[k] = 3 * nc1;
			ia[k+1] = 3 * nc2;
			nia[nr] = 2;
			nx[3 * nc1]++;
			nx[3 * nc2]++;
/*fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%d\n", nr,k, a[k],a[k+1],d[nr]);*/
			nr++;
			
			/* make latitude constraint */
			k = nnz * nr;
			a[k] = -mbna_offsetweight;
			a[k+1] = mbna_offsetweight;
			d[nr] = mbna_offsetweight * tie->offset_y;
			ia[k] = 3 * nc1 + 1;
			ia[k+1] = 3 * nc2 + 1;
			nia[nr] = 2;
			nx[3 * nc1 + 1]++;
			nx[3 * nc2 + 1]++;
/*fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%d\n", nr,k, a[k],a[k+1],d[nr]);*/
			nr++;
			
			/* make depth offset constraint */
			k = nnz * nr;
			a[k] = -mbna_offsetweight;
			a[k+1] = mbna_offsetweight;
			d[nr] = mbna_offsetweight * tie->offset_z_m;
			ia[k] = 3 * nc1 + 2;
			ia[k+1] = 3 * nc2 + 2;
			nia[nr] = 2;
			nx[3 * nc1 + 2]++;
			nx[3 * nc2 + 2]++;
/*fprintf(stderr,"nr:%d k:%d a1:%g a2:%g d:%d\n", nr,k, a[k],a[k+1],d[nr]);*/
			nr++;
			}
		    }
		    
/* print out matrix problem & calculate error */
/*sigma_total = 0.0;
sigma_crossing = 0.0;
fprintf(stderr,"START PROBLEM: smoothweight:%g avg_dtime_d:%g avg_offset:%g\n",
smoothweight,avg_dtime_d,avg_offset);

for (i=0;i<nr;i++)
{
fprintf(stderr,"row %d with %d elements\n", i, nia[i]);
s = 0.0;
for (j=0;j<nia[i];j++)
    {
    k = nnz * i + j;
    s += x[ia[k]] * a[k];
fprintf(stderr,"element %d column %d    a:%g  x:%g  s:%g\n", j, ia[k], a[k], x[ia[k]], x[ia[k]] * a[k]);
    }
sigma_total += (d[i] - s) * (d[i] - s);
fprintf(stderr,"row %d result:  s:%g d:%g sigma_total:%g %g\n\n", i, s, d[i], (d[i] - s) * (d[i] - s), sigma_total);
if (i >= (nr - ndf * ntie))
    sigma_crossing += (d[i] - s) * (d[i] - s);
}
sigma_total = sqrt(sigma_total) / nr;
sigma_crossing = sqrt(sigma_crossing) / ntie;
fprintf(stderr,"INITIAL SIGMA: sigma_total:%g sigma_crossing:%g\n", sigma_total, sigma_crossing);
*/

		/* now do a bunch of test solutions to find what level of smoothing
		 * starts to impact the solution - we want to use this much and no more
		 */
		first = MB_YES;
		last = MB_NO;
		done = MB_NO;
		iter=0;
		smoothfactor = 100.0;
		smoothmin = -1.0;
		smoothmax = -1.0;
		while (done == MB_NO)
		    {
		    /* first change the smoothing weight in the matrix problem */
		    smoothweight = smoothweight * smoothfactor;
		    iter++;
		    for (i=0;i<3*(nfix+ndx+ndx2);i++)
			{
			for (j=0;j<nia[i];j++)
			    {
			    if (nia[i] > 1)
				{
				k = nnz * i + j;
				a[k] *= smoothfactor;
				}
			    }
			}
		
		    /* now solve inverse problem */
		    if (first == MB_YES)
			{
			sprintf(message,"Inverting %dx%d: iter:%d smooth:%.2g",
				nc, nr, iter, smoothweight);
			}
		    else
			{
			sprintf(message,"Inverting %dx%d: iter:%d smooth:%.2g ratio:%.3f iter:%d smooth:%.2g ",
				nc, nr, iter-1, smoothweight_old,
				(sigma_crossing / sigma_crossing_first), iter, smoothweight);
			}
		    do_message_on(message);
		
		    /* compute upper bound on maximum eigenvalue */
		    ncyc = 0;
		    nsig = 0;
		    lspeig(a, ia, nia, nnz, nc, nr, ncyc,
			    &nsig, x, dx, sigma, work, &smax, &err, &sup);
		    supt = smax + err;
		    if (sup > supt)
			supt = sup;
		    if (mbna_verbose > 0)
		    fprintf(stderr, "Initial lspeig: sup:%g smax:%g err:%g supt:%g\n",
			sup, smax, err, supt);
		    ncyc = 16;
		    for (i=0;i<4;i++)
			{
			lspeig(a, ia, nia, nnz, nc, nr, ncyc,
				&nsig, x, dx, sigma, work, &smax, &err, &sup);
			supt = smax + err;
			if (sup > supt)
			    supt = sup;
			if (mbna_verbose > 0)
			fprintf(stderr, "lspeig[%d]: sup:%g smax:%g err:%g supt:%g\n",
			    i, sup, smax, err, supt);
			}
			
		    /* calculate chebyshev factors (errlsq is the theoretical error) */
		    slo = supt / bandwidth;
		    chebyu(sigma, ncycle, supt, slo, work);
		    errlsq = errlim(sigma, ncycle, supt, slo);
		    if (mbna_verbose > 0)
		    fprintf(stderr, "Theoretical error: %f\n", errlsq);
		    if (mbna_verbose > 1)
		    for (i=0;i<ncycle;i++)
			fprintf(stderr, "sigma[%d]: %f\n", i, sigma[i]);
			
		    /* solve the problem */
		    if (first == MB_YES)
		    	{
			for (i=0;i<nc;i++)
			    x[i] = 0.0;
			}
		    lsqup(a, ia, nia, nnz, nc, nr, x, dx, d, 0, NULL, NULL, ncycle, sigma);
	
		    /* output solution */
		    if (mbna_verbose > 1)
		    for (i=0;i<nc/2;i++)
			{
			fprintf(stderr, "i:%d  offsets: %f %f %f  crossings: %d %d %d\n",
				i, x[3*i], x[3*i+1], x[3*i+2], nx[3*i], nx[3*i+1], nx[3*i+2]);
			}
		
		    /* calculate error */
		    sigma_total = 0.0;
		    sigma_crossing = 0.0;
		    for (i=0;i<nr;i++)
			{
			s = 0.0;
			for (j=0;j<nia[i];j++)
			    {
			    k = nnz * i + j;
			    s += x[ia[k]] * a[k];
			    }
			sigma_total += (d[i] - s) * (d[i] - s);
			if (i >= (nr - ndf * ntie))
			    sigma_crossing += (d[i] - s) * (d[i] - s);
			}
		    sigma_total = sqrt(sigma_total / nr);
		    sigma_crossing = sqrt(sigma_crossing / ntie);
		
		    /* keep track of results */
		    if (first == MB_YES)
			{
			first = MB_NO;
			sigma_crossing_first = project.precision * mbna_mtodeglat;
			sigma_crossing_first = MAX(sigma_crossing, sigma_crossing_first);
			smoothweight_old = smoothweight;
			smoothmin = smoothweight;
			}
		    else if (sigma_crossing >= 2.005 * sigma_crossing_first
			    && sigma_crossing <= 2.01 * sigma_crossing_first
			    && sigma_crossing > 0.0)
			{
			/* set to do last inversion with initial data guess set
				to linear interpolation of current model */
			last = MB_YES;
			}
		    else if (last == MB_YES)
			{
			done = MB_YES;
			smoothweight_best = smoothweight;
			sigma_total_best = sigma_total;
			sigma_crossing_best = sigma_crossing;
			}
		    else if (sigma_crossing < 2.005 * sigma_crossing_first)
			{
			if (smoothweight > smoothmin)
			    {
			    smoothmin = smoothweight;
			    }
			if (smoothmax > 0.0)
			    smoothfactor = (smoothmin + 0.3 * (smoothmax - smoothmin))
						/ smoothweight;				
			smoothweight_old = smoothweight;
			}
		    else if (sigma_crossing > 2.01 * sigma_crossing_first
			    && sigma_crossing > 0.0)
			{
			if (smoothweight < smoothmax || smoothmax < 0.0)
			    {
			    smoothmax = smoothweight;
			    }
			smoothfactor = (smoothmin + 0.3 * (smoothmax - smoothmin))
					    / smoothweight;				
			smoothweight_old = smoothweight;
			}
		    else
			{
			smoothweight_old = smoothweight;
			}
		
		    /* do message */
		    sprintf(message, " >   %d %12g %12g %12g %12g\n",
			    iter, smoothweight, sigma_total/ mbna_mtodeglat, 
			    sigma_crossing/ mbna_mtodeglat, 
			    (sigma_crossing / sigma_crossing_first));
		    do_info_add(message, MB_NO);
/* fprintf(stderr,"iteration:%3d smooth:%12g sigmatot:%12g sigmacrossing:%12g ratio:%12g\n",
iter,smoothweight,sigma_total / mbna_mtodeglat, 
sigma_crossing / mbna_mtodeglat,
(sigma_crossing / sigma_crossing_first));*/
		    }

		/* save linear interpolation of solution onto the navigation */
		k = 0;
		nseq = 0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    if (section->continuity == MB_YES)
			    	nseq++;
			    else
			    	nseq = 0;
			    if (section->snav_num_ties[isnav] > 0)
			    	{
				section->snav_lon_offset[isnav] = x[k];
			    	section->snav_lat_offset[isnav] = x[k+1];
			    	section->snav_z_offset[isnav] = x[k+2];
				}
			    else if (k == 0)
			    	{
				section->snav_lon_offset[isnav] = x[k];
			    	section->snav_lat_offset[isnav] = x[k+1];
			    	section->snav_z_offset[isnav] = x[k+2];
				}
			    else if (k == nc)
			    	{
				section->snav_lon_offset[isnav] = x[k-3];
			    	section->snav_lat_offset[isnav] = x[k-2];
			    	section->snav_z_offset[isnav] = x[k-1];
				}
			    else if (section->snav_invert_constraint[isnav] == 0)
			    	{
				section->snav_lon_offset[isnav] = 0.0;
			    	section->snav_lat_offset[isnav] = 0.0;
			    	section->snav_z_offset[isnav] = 0.0;
				}
			    else if (section->snav_invert_constraint[isnav] == 1)
			    	{
				section->snav_lon_offset[isnav] = x[k-3];
			    	section->snav_lat_offset[isnav] = x[k-2];
			    	section->snav_z_offset[isnav] = x[k-1];
				}
			    else if (section->snav_invert_constraint[isnav] == 2)
			    	{
				section->snav_lon_offset[isnav] = x[k];
			    	section->snav_lat_offset[isnav] = x[k+1];
			    	section->snav_z_offset[isnav] = x[k+2];
				}
			    else
			    	{
				factor = (section->snav_time_d[isnav] - xtime_d[(k-3)/3]) / (xtime_d[k/3] - xtime_d[(k-3)/3]);
				section->snav_lon_offset[isnav] = x[k-3] + (x[k] - x[k-3]) * factor;
			    	section->snav_lat_offset[isnav] = x[k-2] + (x[k+1] - x[k-2]) * factor;
			    	section->snav_z_offset[isnav] = x[k-1] + (x[k+2] - x[k-1]) * factor;
				}
			    if (section->snav_num_ties[isnav] > 0)
				k += 3;
			    }
			}
		    }
		    
		/* look for unconstrained sections and set accordingly */
		k = 0;
		i0 = 0;
		i1 = 0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    }
			}
		    }
		
		}
		
	/* output results from navigation solution */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed == project.num_crossings
			|| project.num_truecrossings_analyzed == project.num_truecrossings)
		&& error == MB_ERROR_NO_ERROR)
    		{
		
		/* now output inverse solution */
		sprintf(message,"Outputting navigation solution...");
		do_message_on(message);

		sprintf(message, " > Final smoothing weight:%12g\n > Final crossing sigma:%12g\n > Final total sigma:%12g\n",
			smoothweight_best, sigma_crossing_best / mbna_mtodeglat, sigma_total_best / mbna_mtodeglat);
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

			sprintf(message, " >     %4d   %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f\n",
				icrossing,
				tie->offset_x / mbna_mtodeglon,
				tie->offset_y / mbna_mtodeglat,
				offset_x / mbna_mtodeglon,
				offset_y / mbna_mtodeglat,
				offset_z,
				(offset_x - tie->offset_x) / mbna_mtodeglon,
				(offset_y - tie->offset_y) / mbna_mtodeglat,
				(offset_z - tie->offset_z_m));
			do_info_add(message, MB_NO);
			}
		    }

		if (mbna_verbose > 0)
		for (i=0;i<nc/3;i++)
		    {
		    fprintf(stderr, "i:%d  offsets: %f %f %f  crossings: %d %d %d\n",
		    		i, x[3*i], x[3*i+1], x[3*i+2], nx[3*i], nx[3*i+1], nx[3*i+1]);
		    }
		if (mbna_verbose > 0)
		for (i=0;i<nr;i++)
		    {
		    s = 0.0;
		    for (j=0;j<nia[i];j++)
			{
			k = nnz * i + j;
			s += x[ia[k]] * a[k];
			if (mbna_verbose > 0)
			fprintf(stderr, "i:%4d j:%4d k:%4d ia[k]:%4d a[k]:%12g\n", i, j, k, ia[k], a[k]);
			}
		    if (mbna_verbose > 0)
		    fprintf(stderr, "i:%5d n:%5d  d:%12g s:%12g err:%12g\n",i,nia[i],d[i],s,d[i]-s);
		    }
		
		/* write updated project */
		project.inversion = MBNA_INVERSION_CURRENT;
		mbnavadjust_write_project();

		/* deallocate arrays */
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&a,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ia,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nia,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&d,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nx,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&dx,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&sigma,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&work,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&xtime_d,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&xcontinuity,&error);

		/* generate new nav files */
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    if (file->status != MBNA_FILE_FIXED)
		    	{
		    	sprintf(npath,"%s/nvs_%4.4d.mb166",
			    project.datadir,i);
		    	sprintf(apath,"%s/nvs_%4.4d.na%d",
			    project.datadir,i,file->output_id);
		    	sprintf(opath,"%s.na%d", file->path, file->output_id);
		    	if ((nfp = fopen(npath,"r")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		    	else if ((afp = fopen(apath,"w")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		   	else if ((ofp = fopen(opath,"w")) == NULL)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_OPEN_FAIL;
			    }
		    	else
			    {
			    sprintf(message, " > Output updated nav to %s\n", opath);
			    do_info_add(message, MB_NO);

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
			    	else if ((nscan = sscanf(buffer, "%d %d %d %d %d %d.%d %lf %lf %lf %lf %lf %lf %lf %lf %lf",
							&time_i[0], &time_i[1], &time_i[2], &time_i[3],
							&time_i[4], &time_i[5], &time_i[6], &time_d,
							&navlon, &navlat, &heading, &speed, 
							&draft, &roll, &pitch, &heave)) >= 12)
				    {
				    /* fix nav from early version */
				    if (nscan < 16)
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
					    factor = (time_d - section->snav_time_d[isnav])
							/ (section->snav_time_d[isnav+1] - section->snav_time_d[isnav]);
				    	else
					    factor = 0.0;
				    	}
				    /* update and output only nonzero navigation */
				    if (fabs(navlon) > 0.0000001 && fabs(navlat) > 0.0000001)
				    	{
					navlon -= section->snav_lon_offset_int[isnav]
							+ factor * (section->snav_lon_offset_int[isnav+1]
								- section->snav_lon_offset_int[isnav]);
					navlat -= section->snav_lat_offset_int[isnav]
							+ factor * (section->snav_lat_offset_int[isnav+1]
								- section->snav_lat_offset_int[isnav]);
					zoffset = section->snav_z_offset_int[isnav]
							+ factor * (section->snav_z_offset_int[isnav+1]
								- section->snav_z_offset_int[isnav]);

					/* write the updated nav out */
					fprintf(ofp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						    time_i[0], time_i[1], time_i[2], time_i[3],
						    time_i[4], time_i[5], time_i[6], time_d,
						    navlon, navlat, heading, speed, 
						    draft, roll, pitch, heave, zoffset);
					fprintf(afp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						    time_i[0], time_i[1], time_i[2], time_i[3],
						    time_i[4], time_i[5], time_i[6], time_d,
						    navlon, navlat, heading, speed, 
						    draft, roll, pitch, heave, zoffset);
/*fprintf(stderr, "%2.2d:%2.2d:%2.2d:%5.3f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f\n",
i, isection, isnav, factor, 
time_i[0], time_i[1], time_i[2], time_i[3],
time_i[4], time_i[5], time_i[6], time_d,
navlon, navlat, heading, speed, zoffset);*/
					}
				    }
			    	}
			    }
		    	if (nfp != NULL) fclose(nfp);
		    	if (afp != NULL) fclose(afp);
		    	if (ofp != NULL) 
			    {
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
int mbnavadjust_set_modelplot_graphics(int mp_xgid, int *mp_brdr)
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
		fprintf(stderr,"dbg2       mp_xgid:      %d\n",mp_xgid);
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

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}
		    
	/* plot zoom if active */
	if (mbna_modelplot_zoom_x1 != 0 || mbna_modelplot_zoom_x2 != 0)
		{
		plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
		xo = 5 * MBNA_MODELPLOT_X_SPACE;
		xscale = ((double)plot_width) / (mbna_modelplot_pingend - mbna_modelplot_pingstart + 1);

		ipingstart = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - xo) / xscale + mbna_modelplot_pingstart;
		ipingstart = MIN(MAX(ipingstart, 0), project.num_pings - 1);
		ipingend = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - xo) / xscale + mbna_modelplot_pingstart;
		ipingend = MIN(MAX(ipingend, 0), project.num_pings - 1);

		if (ipingend > ipingstart)
			{
			mbna_modelplot_pingstart = ipingstart;
			mbna_modelplot_pingend = ipingend;
			}
		mbna_modelplot_zoom_x1 = 0;
		mbna_modelplot_zoom_x2 = 0;
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
mbnavadjust_modelpot_pick(int x, int y)
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelpot_pick";
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

			iping = section->global_start_ping + section->snav_id[tie->snav_1];
			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));

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
			

			/* handle second snav point */
			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];

			iping = section->global_start_ping + section->snav_id[tie->snav_2];
			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));

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
				    if (project.modelplot == MB_YES)
					    mbnavadjust_modelplot_plot();
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
mbnavadjust_modelpot_repick(int x, int y)
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelpot_repick";
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
		    
	/* first snav pick had multiple ties - now pick which one to use */
    	if (project.open == MB_YES
    		&& project.inversion != MBNA_INVERSION_NONE
		&& project.modelplot == MB_YES
		&& mbna_modelplot_pickfile != MBNA_SELECT_NONE)
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
				iping = section->global_start_ping + section->snav_id[tie->snav_2];
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));

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
fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);

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
fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);

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
fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);
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
				iping = section->global_start_ping + section->snav_id[tie->snav_1];
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));

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
fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);

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
fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);

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
fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);
				}
			    }
			}
		    }
		    
		/* deal with successful pick */
		if (rangemin < 10000000)
		    {			
		    /* if only one tie go ahead and select it and open it in naverr */
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
			if (project.modelplot == MB_YES)
				mbnavadjust_modelplot_plot();
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
mbnavadjust_modelplot_plot()
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_plot";
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
	int	ipingstart, ipingend;
	int	ixo, iyo, ix, iy;
	int	i, j, isnav;

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
		/* get min maxes by looping over files */
		first = MB_YES;
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			for (isnav=0;isnav<section->num_snav;isnav++)
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
		mbna_modelplot_xscale = ((double)plot_width) / (mbna_modelplot_pingend - mbna_modelplot_pingstart + 1);
		mbna_modelplot_yscale = ((double)plot_height) / (2.2 * xymax);
		yzmax = MAX(fabs(z_offset_min),fabs(z_offset_max));
		yzmax = MAX(yzmax,0.5);
		mbna_modelplot_yzscale = ((double)plot_height) / (2.2 * yzmax);

		/* clear screens for first plot */
		xg_fillrectangle(pmodp_xgid, 0, 0,
				modp_borders[1], modp_borders[3],
				pixel_values[WHITE], XG_SOLIDLINE);
				
		/* plot the bounds */
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon - plot_height / 2, plot_width, plot_height, pixel_values[BLACK], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lon, pixel_values[BLACK], XG_DASHLINE);
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat - plot_height / 2, plot_width, plot_height, pixel_values[BLACK], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lat, pixel_values[BLACK], XG_DASHLINE);
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z - plot_height / 2, plot_width, plot_height, pixel_values[BLACK], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_z, pixel_values[BLACK], XG_DASHLINE);
		
		/* plot labels */
		sprintf(label, "East-West Offset (meters) vs. Ping Count");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_lon - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_pingstart);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_pingend);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);
		
		sprintf(label,"%.2f", 1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);

		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);
		

		sprintf(label, "North-South Offset (meters) vs. Ping Count");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_lat - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_pingstart);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_pingend);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);
		
		sprintf(label,"%.2f", 1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);

		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);
		

		sprintf(label, "Vertical Offset (meters) vs. Ping Count");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_z - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_pingstart);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_z + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_pingend);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_z + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);
		
		sprintf(label,"%.2f", 1.1 * yzmax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);
		
		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * yzmax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[BLACK], XG_SOLIDLINE);
		
		/* set clipping */
		xg_setclip(pmodp_xgid, mbna_modelplot_xo, 0, plot_width, mbna_modelplot_height);
		
		/* Now plot the east-west offsets */
		ixo = 0;
		iyo = 0;
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    iping = section->global_start_ping + section->snav_id[isnav];
			    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));
			    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[isnav] / mbna_mtodeglon);
			    if ((i > 0 || j > 0) && section->continuity == MB_NO && isnav == 0)
		    		xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[3], XG_SOLIDLINE);
			    if (isnav > 0)
		    		xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[2], XG_SOLIDLINE);
			    ixo = ix;
			    iyo = iy;
			    }
			}
		    }
		ixo = 0;
		iyo = 0;
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    iping = section->global_start_ping + section->snav_id[isnav];
			    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));
			    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset_int[isnav] / mbna_mtodeglon);
			    if (isnav > 0)
		    		xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[BLACK], XG_SOLIDLINE);
			    ixo = ix;
			    iyo = iy;
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
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    iping = section->global_start_ping + section->snav_id[isnav];
			    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));
			    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[isnav] / mbna_mtodeglat);
			    if ((i > 0 || j > 0) && section->continuity == MB_NO && isnav == 0)
		    		xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[3], XG_SOLIDLINE);
			    if (isnav > 0)
		    		xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[2], XG_SOLIDLINE);
			    ixo = ix;
			    iyo = iy;
			    }
			}
		    }
		ixo = 0;
		iyo = 0;
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    iping = section->global_start_ping + section->snav_id[isnav];
			    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));
			    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset_int[isnav] / mbna_mtodeglat);
			    if (isnav > 0)
		    		xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[BLACK], XG_SOLIDLINE);
			    ixo = ix;
			    iyo = iy;
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
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    iping = section->global_start_ping + section->snav_id[isnav];
			    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));
			    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[isnav]);
			    if ((i > 0 || j > 0) && section->continuity == MB_NO && isnav == 0)
		    		xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[3], XG_SOLIDLINE);
			    if (isnav > 0)
		    		xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[2], XG_SOLIDLINE);
			    ixo = ix;
			    iyo = iy;
			    }
			}
		    }
		ixo = 0;
		iyo = 0;
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    iping = section->global_start_ping + section->snav_id[isnav];
			    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));
			    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset_int[isnav]);
			    if (isnav > 0)
		    		xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[BLACK], XG_SOLIDLINE);
			    ixo = ix;
			    iyo = iy;
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
			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];
			
			if (tie->inversion_status == MBNA_INVERSION_CURRENT)
				pixel = pixel_values[BLACK];
			else
				pixel = pixel_values[4];
			
			iping = section->global_start_ping + section->snav_id[tie->snav_1];
			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));
			
			iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon);
			xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
			
			iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat);
			xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
			
			iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
			xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);

			tie = &(crossing->ties[j]);
			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];
			iping = section->global_start_ping + section->snav_id[tie->snav_2];
			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));
			
			iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon);
			xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
			
			iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat);
			xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
			
			iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
			xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
			}
		    }
		    
		/* loop over all crossings and plot current tie in red */
		if (mbna_current_crossing != MBNA_SELECT_NONE && mbna_current_tie != MBNA_SELECT_NONE)
		    {
		    for (i=0;i<project.num_crossings;i++)
			{
			crossing = &(project.crossings[i]);
			for (j=0;j<crossing->num_ties;j++)
		    	    {
			    tie = &(crossing->ties[j]);
			    file = &project.files[crossing->file_id_1];
			    section = &file->sections[crossing->section_1];

			    if (i == mbna_current_crossing && j == mbna_current_tie)
				{
				pixel = pixel_values[2];

				iping = section->global_start_ping + section->snav_id[tie->snav_1];
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));

				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[2], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[BLACK], XG_SOLIDLINE);

				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[2], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[BLACK], XG_SOLIDLINE);

				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[2], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[BLACK], XG_SOLIDLINE);

				tie = &(crossing->ties[j]);
				file = &project.files[crossing->file_id_2];
				section = &file->sections[crossing->section_2];
				iping = section->global_start_ping + section->snav_id[tie->snav_2];
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));

				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[2], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[BLACK], XG_SOLIDLINE);

				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[2], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[BLACK], XG_SOLIDLINE);

				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[2], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[BLACK], XG_SOLIDLINE);
				}
			    }
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
				    iping = section->global_start_ping + section->snav_id[tie->snav_1];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));

				    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[2], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);

				    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[2], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);

				    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[2], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);

				    file = &project.files[crossing->file_id_2];
				    section = &file->sections[crossing->section_2];
				    iping = section->global_start_ping + section->snav_id[tie->snav_2];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));

				    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);

				    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);

				    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);
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
				    iping = section->global_start_ping + section->snav_id[tie->snav_2];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));

				    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[2], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);

				    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[2], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);

				    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[2], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);

				    file = &project.files[crossing->file_id_1];
				    section = &file->sections[crossing->section_1];
				    iping = section->global_start_ping + section->snav_id[tie->snav_1];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_pingstart));

				    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);

				    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);

				    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
				    xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
				    xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[BLACK], XG_SOLIDLINE);
				    }
				}
			    }
			}
		    }
		    
		/* plot zoom if active */
		if (mbna_modelplot_zoom_x1 != 0 || mbna_modelplot_zoom_x2 != 0)
			{
			ipingstart = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale + mbna_modelplot_pingstart;
			ipingstart = MIN(MAX(ipingstart, 0), project.num_pings - 1);
			ipingend = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale + mbna_modelplot_pingstart;
			ipingend = MIN(MAX(ipingend, 0), project.num_pings - 1);

			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (ipingstart - mbna_modelplot_pingstart));
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[BLACK], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[BLACK], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[BLACK], XG_DASHLINE);

			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (ipingend - mbna_modelplot_pingstart));
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[BLACK], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[BLACK], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[BLACK], XG_DASHLINE);
			
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

