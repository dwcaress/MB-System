/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust_prog.c	3/23/00
 *    $Id: mbnavadjust_prog.c,v 5.1 2000-12-10 20:29:34 caress Exp $
 *
 *    Copyright (c) 2000 by
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
static char rcs_id[] = "$Id: mbnavadjust_prog.c,v 5.1 2000-12-10 20:29:34 caress Exp $";
static char program_name[] = "mbnavadjust";
static char help_message[] =  "mbnavadjust is an interactive navigation adjustment package for swath sonar data.\n";
static char usage_message[] = "mbnavadjust [-Iproject -V -H]";

/* status variables */
int	error = MB_ERROR_NO_ERROR;
char	message[STRING_MAX];
char	error1[STRING_MAX];
char	error2[STRING_MAX];
char	error3[STRING_MAX];

/* data file parameters */
struct mb_datalist_struct *datalist;
int	format;

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
int	beams_bath;
int	beams_amp;
int	pixels_ss;
char	*imbio_ptr = NULL;
char	*ombio_ptr = NULL;

/* mbio read and write values */
char	*istore_ptr = NULL;
char	*ostore_ptr = NULL;
int	kind;
int	time_i[7];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
double	distance;
double	draft;
double	roll;
double	pitch;
double	heave;
int	nbath;
int	namp;
int	nss;
char	*beamflag = NULL;
double	*bath = NULL;
double	*bathacrosstrack = NULL;
double	*bathalongtrack = NULL;
double	*amp = NULL;
double	*ss = NULL;
double	*ssacrosstrack = NULL;
double	*ssalongtrack = NULL;
int	idata = 0;
int	icomment = 0;
int	odata = 0;
int	ocomment = 0;
char	comment[256];

/* color control values */
#define	WHITE	0	
#define	BLACK	1	
#define RED	2
#define GREEN	3
#define BLUE	4
#define CORAL	5
#define	XG_SOLIDLINE	0
#define	XG_DASHLINE	1
int	cont_xgid;
int	corr_xgid;
int	ncolors;
int	pixel_values[256];

/* Set these to the dimensions of your canvas drawing */
/* areas, minus 1, located in the uil file             */
static int corr_borders[4];
static int cont_borders[4];

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
double	grid_dx = 0.0;
double	grid_dy = 0.0;
double	grid_olon = 0.0;
double	grid_olat = 0.0;
double	misfit_min, misfit_max;
int	gridm_nx = 0;
int	gridm_ny = 0;
int	gridm_nxy = 0;
double	*grid1 = NULL;
double	*grid2 = NULL;
double	*gridm = NULL;
int	*gridn1 = NULL;
int	*gridn2 = NULL;
int	*gridnm = NULL;

/* system function declarations */
char	*ctime();
char	*getenv();
char	*strstr();

/*--------------------------------------------------------------------*/
int mbnavadjust_init_globals()
{
	/* local variables */
	char	*function_name = "mbnavadjust_init_globals";
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
	project.num_crossings = 0;
	project.num_crossings_alloc = 0;
 	project.num_crossings_analyzed = 0;
 	project.crossings = NULL;
	project.num_ties = 0;
	project.logfp = NULL;
 	mbna_status = MBNA_STATUS_GUI;
 	mbna_view_list = MBNA_VIEW_LIST_FILES;
 	project.section_length = 10.0;
 	project.decimation = 1;
 	mbna_current_crossing = -1;
 	mbna_current_tie = -1;
	mbna_total_num_pings = 0;
	mbna_naverr_load = MB_NO;
 	mbna_file_select = MBNA_SELECT_NONE;
	mbna_crossing_select = MBNA_SELECT_NONE;
	mbna_tie_select = MBNA_SELECT_NONE;
	project.cont_int = 25.;
	project.col_int = 100.;
	project.tick_int = 100.;
	mbna_contour_algorithm = MB_CONTOUR_OLD;
	mbna_ncolor = 4;
	mbna_contour = NULL;
	mbna_contour1.nvector = 0;
	mbna_contour1.nvector_alloc = 0;
	mbna_contour1.vector = NULL;
	mbna_contour2.nvector = 0;
	mbna_contour2.nvector_alloc = 0;
	mbna_contour2.vector = NULL;
	mbna_smoothweight = 100.0;
	mbna_offsetweight = 1.0;
	mbna_misfit_center = MBNA_MISFIT_AUTOCENTER;
	mbna_bias_mode = MBNA_BIAS_SAME;

	/* set mbio default values */
	status = mb_defaults(mbna_verbose,&format,&pings,&lonflip,bounds,
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
int mbnavadjust_init(int argc,char **argv,int *startup_file)
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
	while ((c = getopt(argc, argv, "VvHhI:i:")) != -1)
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
		fprintf(stderr,"dbg2       format:          %d\n",format);
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
		/*status = mbnavadjust_action_open(); */
		if (status == MB_SUCCESS)
			*startup_file = MB_YES;
		}
	else
		*startup_file = MB_NO;

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
int mbnavadjust_set_graphics(int cn_xgid, int cr_xgid, 
				int *cn_brdr, int *cr_brdr, 
				int ncol, int *pixels)
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
		fprintf(stderr,"dbg2       cn_brdr:      %d %d %d %d\n",
			cn_brdr[0], cn_brdr[1], cn_brdr[2], cn_brdr[3]);
		fprintf(stderr,"dbg2       cr_brdr:      %d %d %d %d\n",
			cr_brdr[0], cr_brdr[1], cr_brdr[2], cr_brdr[3]);
		fprintf(stderr,"dbg2       ncolors:      %d\n",ncol);
		for (i=0;i<ncol;i++)
			fprintf(stderr,"dbg2       pixel[%d]:     %d\n",
				i, pixels[i]);
		}

	/* set graphics id */
	cont_xgid = cn_xgid;
	corr_xgid = cr_xgid;
	
	/* set borders */
	for (i=0;i<4;i++)
		{
		cont_borders[i] = cn_brdr[i];
		corr_borders[i] = cr_brdr[i];
		}

	/* set colors */
	ncolors = ncol;
	for (i=0;i<ncolors;i++)
		pixel_values[i] = pixels[i];

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
	int	i;

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
				project.num_crossings = 0;
				project.num_crossings_alloc = 0;
				project.crossings = NULL;
				project.num_ties = 0;
				project.inversion = MBNA_INVERSION_NONE;
				
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
	int	i;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	int	i, j;

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
			mb_free(mbna_verbose,&file->sections,&error);
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
	project.num_crossings = 0;
 	project.num_crossings_analyzed = 0;
	project.num_ties = 0;
 	project.inversion = MBNA_INVERSION_NONE;
	mbna_total_num_pings = 0;
	mbna_total_num_snavs = 0;
	
	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
		fprintf(hfp,"##MBNAVADJUST PROJECT\n");
		fprintf(hfp,"MB-SYSTEM_VERSION\t%s\n",MB_VERSION);
		fprintf(hfp,"PROGRAM_VERSION\t%s\n",rcs_id);
		fprintf(hfp,"FILE_VERSION\t1.00\n");
		fprintf(hfp,"NAME\t%s\n",project.name);
		fprintf(hfp,"PATH\t%s\n",project.path);
		fprintf(hfp,"HOME\t%s\n",project.home);
		fprintf(hfp,"DATADIR\t%s\n",project.datadir);
		fprintf(hfp,"NUMFILES\t%d\n",project.num_files);
		fprintf(hfp,"NUMCROSSINGS\t%d\n",project.num_crossings);
		fprintf(hfp,"SECTIONLENGTH\t%f\n",project.section_length);
		fprintf(hfp,"DECIMATION\t%d\n",project.decimation);
		fprintf(hfp,"CONTOURINTERVAL\t%f\n",project.cont_int);
		fprintf(hfp,"COLORINTERVAL\t%f\n",project.col_int);
		fprintf(hfp,"TICKINTERVAL\t%f\n",project.tick_int);
		fprintf(hfp,"INVERSION\t%d\n",project.inversion);
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
				fprintf(hfp,"SECTION %4d %5d %5d %d %d %10.6f %16.6f %16.6f %12.7f %12.7f %12.7f %12.7f %9.3f %9.3f\n",
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
					section->depthmax);
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
				    fprintf(hfp,"SNAV %4d %5d %10.6f %16.6f %12.7f %12.7f %12.7f %12.7f\n",
					    k, 
					    section->snav_id[k],
					    section->snav_distance[k],
					    section->snav_time_d[k],
					    section->snav_lon[k],
					    section->snav_lat[k], 				
					    section->snav_lon_offset[k],
					    section->snav_lat_offset[k]);				
				    }
				}
			}
			
		/* write out crossing info */
		for (i=0;i<project.num_crossings;i++)
			{
			/* write out basic crossing info */
			crossing = &project.crossings[i];
			fprintf(hfp,"CROSSING %5d %1d %5d %3d %5d %3d %2d\n",
				i,
				crossing->status,
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
				fprintf(hfp,"TIE %5d %5d %12.7f %5d %12.7f %12.7f %12.7f %1.1d %12.7f %12.7f\n",
					j,
					tie->snav_1,
					tie->snav_1_time_d,
					tie->snav_2,
					tie->snav_2_time_d,
					tie->offset_x,
					tie->offset_y,
					tie->inversion_status,
					tie->inversion_offset_x,
					tie->inversion_offset_y);
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

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
	int	nscan, idummy;
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
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
				|| strcmp(label,"FILE_VERSION") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,project.name)) != 2
				|| strcmp(label,"NAME") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,project.path)) != 2
				|| strcmp(label,"PATH") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,project.home)) != 2
				|| strcmp(label,"HOME") != 0))
			status = MB_FAILURE;
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,project.datadir)) != 2
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
					nscan = sscanf(buffer,"SECTION %d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf",
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
						&section->depthmax);
				if (result != buffer || nscan != 14)
					{
					status = MB_FAILURE;
fprintf(stderr, "read failed on section: %s\n", buffer);
					}
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
					nscan = sscanf(buffer,"SNAV %d %d %lf %lf %lf %lf %lf %lf",
						&idummy,
						&section->snav_id[k],
						&section->snav_distance[k],
						&section->snav_time_d[k],
						&section->snav_lon[k],
						&section->snav_lat[k],
						&section->snav_lon_offset[k],
						&section->snav_lat_offset[k]);	
				    if (result == buffer && nscan == 6)
				    	{
				    	section->snav_lon_offset[k] = 0.0;
				    	section->snav_lat_offset[k] = 0.0;
				    	}			
				    else if (result != buffer || nscan != 8)
					{
					status = MB_FAILURE;
fprintf(stderr, "read failed on snav: %s\n", buffer);
					}
				    }
				section->global_start_ping = mbna_total_num_pings;
				section->global_start_snav = mbna_total_num_snavs - section->continuity;
				mbna_total_num_pings += section->num_pings;
				mbna_total_num_snavs += (section->num_snav - section->continuity);
				}
			}
			
		/* read crossings */
 		project.num_crossings_analyzed = 0;
 		project.num_ties = 0;
		for (i=0;i<project.num_crossings;i++)
			{
			/* read each crossing */
			crossing = &project.crossings[i];
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
fprintf(stderr, "read failed on crossing: %s\n", buffer);
					}
			if (status == MB_SUCCESS
			    && crossing->status != MBNA_CROSSING_STATUS_NONE)
 				project.num_crossings_analyzed++;
				
			/* read ties */
			if (status == MB_SUCCESS)
			for (j=0;j<crossing->num_ties;j++)
				{
				/* read each tie */
				tie = &crossing->ties[j];
				if (status == MB_SUCCESS
					&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
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
						&tie->inversion_offset_y)) != 10))
						{
						status = MB_FAILURE;
fprintf(stderr, "read failed on tie: %s\n", buffer);
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
				free(project.files);
			project.open = MB_NO;
			memset(project.name,0,STRING_MAX);
			strcpy(project.name,"None");
 			memset(project.path,0,STRING_MAX);
			memset(project.datadir,0,STRING_MAX);
			project.num_files = 0;
			project.num_files_alloc = 0;
			project.num_crossings = 0;
			project.num_crossings_alloc = 0;
 			project.num_crossings_analyzed = 0;
			project.num_ties = 0;
 			}
		}	
	
	/* else set error */
	else
		{
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_import_data(char *path, int format)
{
	/* local variables */
	char	*function_name = "mbnavadjust_import_data";
	int	status = MB_SUCCESS;
	int	done;
	char	file[STRING_MAX];
	int	form;
	double	weight;
 	int	i, j;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2               path:     %s\n",path);
		fprintf(stderr,"dbg2               format:   %d\n",format);
		}
	
	/* loop until all files read */
	done = MB_NO;
	while (done == MB_NO)
		{
		if (format > 0)
			{
			status = mbnavadjust_import_file(path,format);
			done = MB_YES;
			}
		else if (format == -1)
			{
			if (status = mb_datalist_open(mbna_verbose,&datalist,
							path,&error) == MB_SUCCESS)
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_import_file(char *path, int format)
{
	/* local variables */
	char	*function_name = "mbnavadjust_import_file";
	int	status = MB_SUCCESS;
	int	done;
	struct stat file_status;
	int	fstat;
	char	ipath[STRING_MAX];
	char	mb_suffix[STRING_MAX];
	char	npath[STRING_MAX];
	char	opath[STRING_MAX];
	int	output_id, found;
	int	obeams_bath,obeams_amp,opixels_ss;
	int	iform;
	double	good_depth;
	int	nread, first;
	int	output_open = MB_NO;
	int	good_bath, good_beams, new_segment;
	int	disqualify;
	double	headingx, headingy, mtodeglon, mtodeglat;
	double	lon, lat, depth;
	double	navlon_old, navlat_old;
	char	*error_message;
	FILE	*nfp;
	struct mbna_file *file, *cfile;
	struct mbna_crossing *crossing;
	struct mbna_section *section, *csection;
	struct mbsys_ldeoih_struct *ostore;
	struct mb_io_struct *omb_io_ptr;
	int	new_pings, new_crossings;
	int	decimate_count;
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
 	int	i, j, k;
	int	ii1, jj1, kk1, ii2, jj2, kk2;
	

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2               path:     %s\n",path);
		fprintf(stderr,"dbg2               format:   %d\n",format);
		}
		
	/* get potential processed file name */
	if ((status = mb_get_format(mbna_verbose, path, ipath, 
				    &iform, &error))
				    == MB_SUCCESS
	    && iform == format)
	    {
	    strcat(ipath,"p");
	    sprintf(mb_suffix, ".mb%d", format);
	    strcat(ipath,mb_suffix);
	    }

	/* else just add p.mbXXX to file name */
	else
		{
		strcat(ipath,"p");
		sprintf(mb_suffix, ".mb%d", format);
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
	 *   a unique nav file file for this input file */
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
	sprintf(message,"Importing data in format %d from %s",format,ipath);
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
	
	if (status = MB_SUCCESS)
		{
		/* initialize reading the swath file */
		if ((status = mb_read_init(
			mbna_verbose,ipath,format,pings,lonflip,bounds,
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
		status = mb_malloc(mbna_verbose,beams_bath*sizeof(char),&beamflag,&error);
		status = mb_malloc(mbna_verbose,beams_bath*sizeof(double),&bath,&error);
		status = mb_malloc(mbna_verbose,beams_bath*sizeof(double),
			&bathacrosstrack,&error);
		status = mb_malloc(mbna_verbose,beams_bath*sizeof(double),
			&bathalongtrack,&error);
		status = mb_malloc(mbna_verbose,beams_amp*sizeof(double),&amp,&error);
		status = mb_malloc(mbna_verbose,pixels_ss*sizeof(double),&ss,&error);
		status = mb_malloc(mbna_verbose,pixels_ss*sizeof(double),&ssacrosstrack,
			&error);
		status = mb_malloc(mbna_verbose,pixels_ss*sizeof(double),&ssalongtrack,
			&error);

		/* if error initializing memory then don't read the file */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(mbna_verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
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
		decimate_count = 0;
		while (error <= MB_ERROR_NO_ERROR)
			{
			/* read a ping of data */
			status = mb_get_all(mbna_verbose,imbio_ptr,&istore_ptr,&kind,
				time_i,&time_d,&navlon,&navlat,&speed,
				&heading,&distance,
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
				strcpy(file->file,path);
				file->format = format;
				file->heading_bias = 0.0;
				file->roll_bias = 0.0;
				file->num_sections = 0;
				file->num_sections_alloc = 0;
				file->sections = NULL;
				project.num_files++;
				new_segment = MB_YES;
				first = MB_NO;
				
				/* get bias values */
				mb_pr_get_heading(mbna_verbose, file->file, 
						    &mbp_heading_mode, 
						    &mbp_headingbias, 
						    &error);
				mb_pr_get_rollbias(mbna_verbose, file->file, 
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
				&& section->distance + distance
					>= project.section_length)
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
					section->num_snav++;
					mbna_total_num_snavs++;
					}
				else if (section->num_snav > 1)
					{
					section->snav_id[section->num_snav-1]
						= section->num_pings - 1;
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
/*fprintf(stderr,"New section: %d\n",file->num_sections);*/
				decimate_count = 0;
				file->num_sections++;
				section = &file->sections[file->num_sections-1];
				section->num_pings = 0;
				section->num_beams = 0;
				section->continuity = MB_NO;
				section->global_start_ping = mbna_total_num_pings;
				section->global_start_snav = mbna_total_num_snavs;
				for (i=0;i<MBNA_MASK_DIM*MBNA_MASK_DIM;i++)
				    section->coverage[i] = 0;
				section->num_snav = 0;
				if (file->num_sections > 1)
					{
					csection = &file->sections[file->num_sections-2];
					if (time_d - csection->etime_d >= 0.0
						&& time_d - csection->etime_d < MBNA_TIME_GAP_MAX)
						{
						section->continuity = MB_YES;
						section->global_start_snav--;
						mbna_total_num_snavs--;
						}
					}
				else if (project.num_files > 1)
					{
					cfile = &project.files[project.num_files-2];
					csection = &cfile->sections[cfile->num_sections-1];
					if (time_d - csection->etime_d >= 0.0
						&& time_d - csection->etime_d < MBNA_TIME_GAP_MAX)
						{
						section->continuity = MB_YES;
						section->global_start_snav--;
						mbna_total_num_snavs--;
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
					status = mb_malloc(mbna_verbose,obeams_bath*sizeof(char),
							&ostore->beamflag,&error);
					status = mb_malloc(mbna_verbose,obeams_bath*sizeof(double),
							&ostore->bath,&error);
					status = mb_malloc(mbna_verbose,obeams_bath*sizeof(double),
							&ostore->bath_acrosstrack,&error);
					status = mb_malloc(mbna_verbose,obeams_bath*sizeof(double),
							&ostore->bath_alongtrack,&error);

					/* if error initializing memory then don't write the file */
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(mbna_verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
							message);
						status = mb_free(mbna_verbose,&ostore->beamflag,&error);
						status = mb_free(mbna_verbose,&ostore->bath,&error);
						status = mb_free(mbna_verbose,&ostore->bath_acrosstrack,&error);
						status = mb_free(mbna_verbose,&ostore->bath_alongtrack,&error);
						status = mb_close(mbna_verbose,&ombio_ptr,&error);
						output_open = MB_NO;
						}
					}			
				}
				
			/* augment decimation count */
			if (good_bath == MB_YES)
				{
				decimate_count++;
				}
				
			/* update section distance for each data ping */
			if (good_bath == MB_YES
				&& section->num_pings > 1)
				section->distance += distance;
				
			/* handle good bathymetry when decimation satisfied */
			if (good_bath == MB_YES
				&& decimate_count == 1)
				{
				/* get statistics */
				mb_coor_scale(mbna_verbose,navlat,&mtodeglon,&mtodeglat);
				headingx = sin(DTR*heading);
				headingy = cos(DTR*heading);
				navlon_old = navlon;
				navlat_old = navlat;
				section->etime_d = time_d;
				section->num_pings++;
				mbna_total_num_pings++;
				new_pings++;
				if (section->distance >= 
				    section->num_snav * project.section_length / (MBNA_SNAV_NUM - 1))
					{
					section->snav_id[section->num_snav]
						= section->num_pings - 1;
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
					section->num_snav++;
					mbna_total_num_snavs++;
					}
				for (i=0;i<beams_bath;i++)
					{
					if (mb_beam_ok(beamflag[i]) && bath[i] != 0.0)
						{
						good_bath = MB_YES;
						good_beams++;
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
				
			/* reset decimation */
			if (good_bath == MB_YES 
				&& decimate_count >= project.decimation)
				decimate_count = 0;
				
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
			status = mb_free(mbna_verbose,&ostore->beamflag,&error);
			status = mb_free(mbna_verbose,&ostore->bath,&error);
			status = mb_free(mbna_verbose,&ostore->bath_acrosstrack,&error);
			status = mb_free(mbna_verbose,&ostore->bath_alongtrack,&error);
			status = mb_close(mbna_verbose,&ombio_ptr,&error);
			}

		/* deallocate memory used for data arrays */
		mb_free(mbna_verbose,&beamflag,&error);
		mb_free(mbna_verbose,&bath,&error);
		mb_free(mbna_verbose,&bathacrosstrack,&error);
		mb_free(mbna_verbose,&bathalongtrack,&error);
		mb_free(mbna_verbose,&amp,&error);
		mb_free(mbna_verbose,&ss,&error);
		mb_free(mbna_verbose,&ssacrosstrack,&error);
		mb_free(mbna_verbose,&ssalongtrack,&error);
		
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
				    status = mb_malloc(mbna_verbose,beams_bath*sizeof(char),&beamflag,&error);
				    status = mb_malloc(mbna_verbose,beams_bath*sizeof(double),&bath,&error);
				    status = mb_malloc(mbna_verbose,beams_bath*sizeof(double),
					    &bathacrosstrack,&error);
				    status = mb_malloc(mbna_verbose,beams_bath*sizeof(double),
					    &bathalongtrack,&error);
				    status = mb_malloc(mbna_verbose,beams_amp*sizeof(double),&amp,&error);
				    status = mb_malloc(mbna_verbose,pixels_ss*sizeof(double),&ss,&error);
				    status = mb_malloc(mbna_verbose,pixels_ss*sizeof(double),&ssacrosstrack,
					    &error);
				    status = mb_malloc(mbna_verbose,pixels_ss*sizeof(double),&ssalongtrack,
					    &error);
		    
				    /* if error initializing memory then don't read the file */
				    if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(mbna_verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
						message);
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
					    &heading,&distance,
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
				status = mb_free(mbna_verbose,&beamflag,&error);
				status = mb_free(mbna_verbose,&bath,&error);
				status = mb_free(mbna_verbose,&bathacrosstrack,&error);
				status = mb_free(mbna_verbose,&bathalongtrack,&error);
				status = mb_free(mbna_verbose,&amp,&error);
				status = mb_free(mbna_verbose,&ss,&error);
				status = mb_free(mbna_verbose,&ssacrosstrack,&error);
				status = mb_free(mbna_verbose,&ssalongtrack,&error);
				
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
					    overlap = MB_NO;
					    for (ii1=0;ii1<MBNA_MASK_DIM && overlap==MB_NO;ii1++)
					    for (jj1=0;jj1<MBNA_MASK_DIM && overlap==MB_NO;jj1++)
						{
						kk1 = ii1 + jj1 * MBNA_MASK_DIM;
						if (section->coverage[kk1] == 1)
						    {
						    lon1min = section->lonmin + dx1 * ii1;
						    lon1max = section->lonmin + dx1 * (ii1 + 1);
						    lat1min = section->latmin + dy1 * jj1;
						    lat1max = section->latmin + dy1 * (jj1 + 1);
						    for (ii2=0;ii2<MBNA_MASK_DIM && overlap==MB_NO;ii2++)
						    for (jj2=0;jj2<MBNA_MASK_DIM && overlap==MB_NO;jj2++)
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
								overlap = MB_YES;
							    }
							}
						    }
						}
					    if (overlap == MB_NO)
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
					    crossing->file_id_1 = file->id;
					    crossing->section_1 = k;
					    crossing->file_id_2 = cfile->id;
					    crossing->section_2 = j;
					    crossing->num_ties = 0;
					    project.num_crossings++;
					    new_crossings++;
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
			format, path, new_pings, file->num_sections, new_crossings);
		do_info_add(message, MB_YES);
		}
	else
		{
		sprintf(message, "Unable to import format %d file: %s\n", 
			format, path);
		do_info_add(message, MB_YES);
		}
	
	/* turn off message */
	do_message_off();
			
 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
			    project.num_crossings_analyzed++;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
/*fprintf(stderr, "tie %d of crossing %d saved...\n", mbna_current_tie, mbna_current_crossing);*/
		    tie = &crossing->ties[mbna_current_tie];
		    tie->snav_1 = mbna_snav_1;
		    tie->snav_1_time_d = mbna_snav_1_time_d;
		    tie->snav_2 = mbna_snav_2;
		    tie->snav_2_time_d = mbna_snav_2_time_d;
		    if (tie->inversion_status == MBNA_INVERSION_CURRENT
			&& (tie->offset_x != mbna_offset_x
			    || tie->offset_y != mbna_offset_y))
			{
			tie->inversion_status = MBNA_INVERSION_OLD;
			}
		    tie->offset_x = mbna_offset_x;
		    tie->offset_y = mbna_offset_y;
		    tie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
		    tie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
		    if (project.inversion == MBNA_INVERSION_CURRENT)
			    project.inversion = MBNA_INVERSION_OLD;
		    }
		}
			
 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
 	int	i;

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
		/* save offsets if ties set */
    		mbnavadjust_naverr_save();
		
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
		/* save offsets if ties set */
		mbnavadjust_naverr_save();
				
    		/* get next crossing */
    		if (mbna_current_crossing >= project.num_crossings - 1)
    			mbna_current_crossing = 0;
    		else
    		 	mbna_current_crossing++;
    		 	
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
		/* save offsets if ties set */
		mbnavadjust_naverr_save();
		
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
  	int	i, j, k;

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
		/* save offsets if ties set */
		mbnavadjust_naverr_save();
		
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
    			if (crossing->status == MBNA_CROSSING_STATUS_NONE)
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
    				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
	int	found;
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
		    && project.crossings[mbna_current_crossing].num_ties > 0)
    			{
			/* save offsets of last tie */
    			mbnavadjust_naverr_save();

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
			file1 = (struct mbna_file *) &project.files[mbna_file_id_1];
			file2 = (struct mbna_file *) &project.files[mbna_file_id_2];
			section1 = (struct mbna_section *) &file1->sections[mbna_section_1];
			section2 = (struct mbna_section *) &file2->sections[mbna_section_2];
			mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2]
						- section1->snav_lon_offset[mbna_snav_1];
			mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2]
						- section1->snav_lat_offset[mbna_snav_1];
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
			/* save offsets of last tie */
    			mbnavadjust_naverr_save();

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
				project.num_crossings_analyzed++;
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
			file1 = (struct mbna_file *) &project.files[mbna_file_id_1];
			file2 = (struct mbna_file *) &project.files[mbna_file_id_2];
			section1 = (struct mbna_section *) &file1->sections[mbna_section_1];
			section2 = (struct mbna_section *) &file2->sections[mbna_section_2];
			mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2]
						- section1->snav_lon_offset[mbna_snav_1];
			mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2]
						- section1->snav_lat_offset[mbna_snav_1];
			tie->inversion_status = MBNA_INVERSION_NONE;
    			tie->inversion_offset_x = mbna_invert_offset_x;
    			tie->inversion_offset_y = mbna_invert_offset_y;
    			tie->inversion_offset_x_m = mbna_invert_offset_x / mbna_mtodeglon;
    			tie->inversion_offset_y_m = mbna_invert_offset_y / mbna_mtodeglat;
   			if (project.inversion == MBNA_INVERSION_CURRENT)
    				project.inversion = MBNA_INVERSION_OLD;
		
			/* write updated project */
			mbnavadjust_write_project();
	
			/* add info text */
			sprintf(message,"Add Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f m\n",
				mbna_current_tie, mbna_current_crossing,
				crossing->file_id_1, crossing->section_1, tie->snav_1,
				crossing->file_id_2, crossing->section_2, tie->snav_2,
				tie->offset_x_m, tie->offset_y_m);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
	int	found;
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
			sprintf(message,"Delete Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f m\n",
				mbna_current_tie, mbna_current_crossing,
				crossing->file_id_1, crossing->section_1, tie->snav_1,
				crossing->file_id_2, crossing->section_2, tie->snav_2,
				tie->offset_x_m, tie->offset_y_m);
			do_info_add(message, MB_YES);

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
			file1 = (struct mbna_file *) &project.files[mbna_file_id_1];
			file2 = (struct mbna_file *) &project.files[mbna_file_id_2];
			section1 = (struct mbna_section *) &file1->sections[mbna_section_1];
			section2 = (struct mbna_section *) &file2->sections[mbna_section_2];
			mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2]
						- section1->snav_lon_offset[mbna_snav_1];
			mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2]
						- section1->snav_lat_offset[mbna_snav_1];
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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

			/* reset offsets */
    			crossing = &project.crossings[mbna_current_crossing];
    			tie = &crossing->ties[mbna_current_tie];
    			mbna_offset_x = tie->offset_x;
    			mbna_offset_y = tie->offset_y;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
    		if (mbna_current_crossing >= 0)
    			{
    			crossing = &project.crossings[mbna_current_crossing];
			project.num_ties -= crossing->num_ties;
			crossing->num_ties = 0;
			if (crossing->status == MBNA_CROSSING_STATUS_NONE)
				project.num_crossings_analyzed++;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
	char	path1[STRING_MAX], path2[STRING_MAX];
	int	done, pings_read;
	char	*error_message;
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
		status = mbnavadjust_crossing_unload();
		}
		
     	/* load current crossing */
    	if (mbna_status == MBNA_STATUS_NAVERR
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
		    file1 = (struct mbna_file *) &project.files[mbna_file_id_1];
		    file2 = (struct mbna_file *) &project.files[mbna_file_id_2];
		    section1 = (struct mbna_section *) &file1->sections[mbna_section_1];
		    section2 = (struct mbna_section *) &file2->sections[mbna_section_2];
		    mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2]
					    - section1->snav_lon_offset[mbna_snav_1];
		    mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2]
					    - section1->snav_lat_offset[mbna_snav_1];
		    }
		else
		    {
		    mbna_offset_x = 0.0;
		    mbna_offset_y = 0.0;
		    }
		sprintf(path1,"%s/nvs_%4.4d_%4.4d.mb71",
			project.datadir,mbna_file_id_1,mbna_section_1);
		sprintf(path2,"%s/nvs_%4.4d_%4.4d.mb71",
			project.datadir,mbna_file_id_2,mbna_section_2);
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
		status = mbnavadjust_section_load(path1, &swathraw1, &swath1, section1->num_pings);
		status = mbnavadjust_section_load(path2, &swathraw2, &swath2, section2->num_pings);
			
		/* get lon lat positions for soundings */
		status = mbnavadjust_section_translate(mbna_file_id_1, swathraw1, swath1);
		status = mbnavadjust_section_translate(mbna_file_id_2, swathraw2, swath2);
	
		/* generate contour data */
		status = mbnavadjust_section_contour(swath1,&mbna_contour1);
		status = mbnavadjust_section_contour(swath2,&mbna_contour2);
		
		/* generate misfit grids */
		status = mbnavadjust_get_misfit();

		/* set loaded flag */
		mbna_naverr_load = MB_YES;
	
		/* turn off message */
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
			status = mb_free(mbna_verbose,&pingraw->beamflag, &error);
			status = mb_free(mbna_verbose,&pingraw->bath, &error);
			status = mb_free(mbna_verbose,&pingraw->bathacrosstrack, &error);
			status = mb_free(mbna_verbose,&pingraw->bathalongtrack, &error);
			}
		    status = mb_free(mbna_verbose,&swathraw1->pingraws, &error);
		    }
		if (swathraw2 != NULL && swathraw2->pingraws != NULL)
		    {
		    for (i=0;i<swathraw2->npings_max;i++)
			{
			pingraw = &swathraw2->pingraws[i];
			status = mb_free(mbna_verbose,&pingraw->beamflag, &error);
			status = mb_free(mbna_verbose,&pingraw->bath, &error);
			status = mb_free(mbna_verbose,&pingraw->bathacrosstrack, &error);
			status = mb_free(mbna_verbose,&pingraw->bathalongtrack, &error);
			}
		    status = mb_free(mbna_verbose,&swathraw2->pingraws, &error);
		    }
		if (swathraw1 != NULL)
		    status = mb_free(mbna_verbose,&swathraw1, &error);
		if (swathraw2 != NULL)
		    status = mb_free(mbna_verbose,&swathraw2, &error);

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
		gridm_nx = 0;
		gridm_ny = 0;
		gridm_nxy = 0;
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
		gridn1 = NULL;
		gridn2 = NULL;
		gridnm = NULL;
		}
			
 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
	struct mbna_crossing *crossing;
	struct mbna_file *file1, *file2;
	struct mbna_section *section1, *section2;
	char	path1[STRING_MAX], path2[STRING_MAX];
	int	done, pings_read;
	char	*error_message;
   	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}
		
	/* replot loaded crossing */
	if (mbna_naverr_load == MB_YES)
		{
    		/* reset contouring parameters */
		swath1->contour_int = project.cont_int;
		swath1->color_int = project.col_int;
		swath1->tick_int = project.tick_int;
		swath2->contour_int = project.cont_int;
		swath2->color_int = project.col_int;
		swath2->tick_int = project.tick_int;
			
		/* get lon lat positions for soundings */
		status = mbnavadjust_section_translate(mbna_file_id_1, swathraw1, swath1);
		status = mbnavadjust_section_translate(mbna_file_id_2, swathraw2, swath2);
	
		/* generate contour data */
		status = mbnavadjust_section_contour(swath1,&mbna_contour1);
		status = mbnavadjust_section_contour(swath2,&mbna_contour2);
   		}
			
 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_section_load(char *path, void **swathraw_ptr, void **swath_ptr, int num_pings)
{
	/* local variables */
	char	*function_name = "mbnavadjust_section_load";
	int	status = MB_SUCCESS;
	struct mb_io_struct *imb_io_ptr;
	struct swathraw *swathraw;
	struct pingraw *pingraw;
	struct swath *swath;
	double	tick_len_map, label_hgt_map;
	int	done, pings_read;
	double	mtodeglon, mtodeglat, headingx, headingy;
	char	*error_message;
   	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       path:         %s\n",path);
		fprintf(stderr,"dbg2       swath_ptr:    %d  %d\n",swath_ptr, *swath_ptr);
		fprintf(stderr,"dbg2       num_pings:    %d\n",num_pings);
		}
		
     	/* load current crossing */
    	if (mbna_status == MBNA_STATUS_NAVERR
		&& project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
    		{		
		/* set section format */
		format = 71;

		/* initialize section for reading */
		if ((status = mb_read_init(
			mbna_verbose,path,format,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&imbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(mbna_verbose,error,&error_message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",error_message);
			fprintf(stderr,"\nSwath sonar File <%s> not initialized for reading\n",path);
			}
	
		/* allocate memory for data arrays */
		if (status == MB_SUCCESS)
			{
			/* get mb_io_ptr */			
			imb_io_ptr = (struct mb_io_struct *) imbio_ptr;
			
			/* initialize data storage */
			status = mb_malloc(mbna_verbose, sizeof(struct swathraw), 
						swathraw_ptr, &error);
			swathraw = (struct swathraw *) *swathraw_ptr;
			swathraw->beams_bath = beams_bath;
			swathraw->npings_max = num_pings;
			swathraw->npings = 0;
			status = mb_malloc(mbna_verbose, num_pings * sizeof(struct pingraw), 
						&swathraw->pingraws, &error);
			for (i=0;i<swathraw->npings_max;i++)
				{
				pingraw = &swathraw->pingraws[i];
				pingraw->beamflag = NULL;
				pingraw->bath = NULL;
				pingraw->bathacrosstrack = NULL;
				pingraw->bathalongtrack = NULL;
				status = mb_malloc(mbna_verbose, beams_bath * sizeof(char), 
							&pingraw->beamflag, &error);
				status = mb_malloc(mbna_verbose, beams_bath * sizeof(double), 
							&pingraw->bath, &error);
				status = mb_malloc(mbna_verbose, beams_bath * sizeof(double), 
							&pingraw->bathacrosstrack, &error);
				status = mb_malloc(mbna_verbose, beams_bath * sizeof(double), 
							&pingraw->bathalongtrack, &error);
				}

			/* initialize contour controls */
			tick_len_map = MAX(mbna_lon_max - mbna_lon_min,
						mbna_lat_max - mbna_lat_min) / 500;
			label_hgt_map = MAX(mbna_lon_max - mbna_lon_min,
						mbna_lat_max - mbna_lat_min) / 100;
 			status = mb_contour_init(mbna_verbose, (struct swath **)swath_ptr,
					    num_pings,
					    beams_bath,
					    mbna_contour_algorithm,
					    MB_YES,MB_NO,MB_NO,
					    project.cont_int, project.col_int,
					    project.tick_int, 1000000.,
					    tick_len_map, label_hgt_map,
					    mbna_ncolor, 0, NULL, NULL, NULL,
					    0.0, 0.0, 0.0, 0.0,
					    &error);
			swath = (struct swath *) *swath_ptr;
			swath->beams_bath = beams_bath;
			swath->npings = 0;

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR)
				{
				mb_error(mbna_verbose,error,&message);
				fprintf(stderr,"\nMBIO Error allocating contour control structure:\n%s\n",message);
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
			    pingraw = &swathraw->pingraws[swathraw->npings];
			    ping = &swath->pings[swath->npings];
			    status = mb_get_all(mbna_verbose,imbio_ptr,
				    &istore_ptr,&kind,
				    pingraw->time_i, &pingraw->time_d,
				    &pingraw->navlon, &pingraw->navlat, &speed,
				    &pingraw->heading, &distance,
				    &beams_bath, &beams_amp, &pixels_ss,
				    pingraw->beamflag, pingraw->bath, 
				    imb_io_ptr->amp,
				    pingraw->bathacrosstrack, pingraw->bathalongtrack,
				    imb_io_ptr->ss, 
				    imb_io_ptr->ss_acrosstrack, 
				    imb_io_ptr->ss_alongtrack,
				    comment, &error);
	
			    /* handle successful read */
			    if (status == MB_SUCCESS
				&& kind == MB_DATA_DATA)
			    	{
			    	/* update bookkeeping */
			    	if (error == MB_ERROR_NO_ERROR)
				    {
				    swathraw->npings++;
				    swath->npings++;
				    }

				/* extract all nav values */
				status = mb_extract_nav(mbna_verbose,imbio_ptr,
					istore_ptr,&kind,
					pingraw->time_i, &pingraw->time_d,
					&pingraw->navlon, &pingraw->navlat, &speed,
					&pingraw->heading, &pingraw->draft,
					&roll, &pitch, &heave, 
					&error);

				/* copy data from swathraw to swath */
				for (i=0;i<7;i++)
				    ping->time_i[i] = pingraw->time_i[i];
				ping->time_d = pingraw->time_d;
				ping->navlon = pingraw->navlon;
				ping->navlat = pingraw->navlat;
				ping->heading = pingraw->heading;
				mb_coor_scale(mbna_verbose, pingraw->navlat, 
						&mtodeglon, &mtodeglat);
				headingx = sin(pingraw->heading * DTR);
				headingy = cos(pingraw->heading * DTR);
			    	for (i=0;i<beams_bath;i++)
				    {
				    ping->beamflag[i] = pingraw->beamflag[i];
				    ping->bath[i] = pingraw->bath[i];
				    ping->bathlon[i] = pingraw->navlon 
							+ headingy*mtodeglon
							    *pingraw->bathacrosstrack[i]
							+ headingx*mtodeglon
							    *pingraw->bathalongtrack[i];
				    ping->bathlat[i] = pingraw->navlat 
							- headingx*mtodeglat
							    *pingraw->bathacrosstrack[i]
							+ headingy*mtodeglat
							    *pingraw->bathalongtrack[i];
				    }

			    	/* null out any unused beams for formats with
					variable numbers of beams */
			    	for (i=beams_bath;i<swathraw->beams_bath;i++)
				    pingraw->beamflag[i] = MB_FLAG_NULL;
			    	for (i=beams_bath;i<swath->beams_bath;i++)
				    ping->beamflag[i] = MB_FLAG_NULL;
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
			}
		
		/* close the input data file */
		status = mb_close(mbna_verbose,&imbio_ptr,&error);
   		}
			
 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_section_translate(int file_id, void *swathraw_ptr, void *swath_ptr)
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
		}
		
     	/* translate sounding positions for loaded section */
    	if (mbna_status == MBNA_STATUS_NAVERR
		&& project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
    		{
		swathraw = (struct swathraw *) swathraw_ptr;
		swath = (struct swath *) swath_ptr;
		
		/* relocate soundings based on heading bias */
		swath->npings = swathraw->npings;
		for (iping=0;iping<swathraw->npings;iping++)
		    {
		    pingraw = &swathraw->pingraws[iping];
		    ping = &swath->pings[iping];
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
		    for (i=0;i<swathraw->beams_bath;i++)
			{
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_section_contour(struct swath *swath,
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
		fprintf(stderr,"dbg2       swath:        %d\n",swath);
		fprintf(stderr,"dbg2       contour:      %d\n",contour);
		fprintf(stderr,"dbg2       nvector:      %d\n",contour->nvector);
		fprintf(stderr,"dbg2       nvector_alloc:%d\n",contour->nvector_alloc);
		}

	if (swath != NULL)
		{
		/* set vectors */
		mbna_contour = contour;
		mbna_contour->nvector = 0;
		
		/* plot data */
		status = mb_contour(mbna_verbose,swath,&error);
		}
			
 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
	int	closest_ping;
	double	closest_time_d;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	struct mbna_section *section;
	int	i, j;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
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
	double	x, y;
	int	igx, igy;
	int	ic, jc, kc;
	int	ioff, joff, istart, iend, jstart, jend;
	int	i1, i2, j1, j2, k1, k2;
	int	i, j, k;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}
		
    	if (mbna_status == MBNA_STATUS_NAVERR
		&& project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
    		{
    		/* set message on */
    		if (mbna_verbose > 1)
			fprintf(stderr,"Making misfit grid for crossing %d\n",mbna_current_crossing);
		sprintf(message,"Making misfit grid for crossing %d\n",mbna_current_crossing);
		do_message_on(message);
		
		grid_nx = 61;
		grid_ny = 61;
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
		gridm_nx = grid_nx / 2 + 1;
		gridm_ny = gridm_nx;
		gridm_nxy = gridm_nx * gridm_ny;
		if (mbna_misfit_center == MBNA_MISFIT_ZEROCENTER)
		    {
		    mbna_misfit_offset_x = 0.0;
		    mbna_misfit_offset_y = 0.0;
		    }
		else
		    {
		    mbna_misfit_offset_x = mbna_offset_x;
		    mbna_misfit_offset_y = mbna_offset_y;
		    }
/*fprintf(stderr, "GRID parameters: dx:%f nx:%d ny:%d  bounds:  grid: %f %f %f %f  plot: %f %f %f %f\n",
grid_dx, grid_nx, grid_ny,
grid_olon, grid_olon + grid_nx * grid_dx,
grid_olat, grid_olat + grid_ny * grid_dy,
mbna_lon_min, mbna_lon_max, mbna_lat_min, mbna_lat_max);*/

		/* allocate and initialize grids */
		grid1 = (double *) realloc(grid1, sizeof(double) * (grid_nxy));
		grid2 = (double *) realloc(grid2, sizeof(double) * (grid_nxy));
		gridm = (double *) realloc(gridm, sizeof(double) * (gridm_nxy));
		gridn1 = (int *) realloc(gridn1, sizeof(int) * (grid_nxy));
		gridn2 = (int *) realloc(gridn2, sizeof(int) * (grid_nxy));
		gridnm = (int *) realloc(gridnm, sizeof(int) * (gridm_nxy));
		memset(grid1, 0, sizeof(double) * (grid_nxy));
		memset(grid2, 0, sizeof(double) * (grid_nxy));
		memset(gridm, 0, sizeof(double) * (gridm_nxy));
		memset(gridn1, 0, sizeof(int) * (grid_nxy));
		memset(gridn2, 0, sizeof(int) * (grid_nxy));
		memset(gridnm, 0, sizeof(int) * (gridm_nxy));
	    	
	    	/* loop over all beams */
	    	for (i=0;i<swath1->npings;i++)
	    		{
	    		for (j=0;j<swath1->beams_bath;j++)
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
	    		for (j=0;j<swath2->beams_bath;j++)
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
i, j, swath2->pings[i].bathlon[j], swath1->pings[i].bathlat[j], x, y, igx, igy);*/					
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
			
		/* calculate gridded misfit over offsets */
		misfit_min = 0.0;
		misfit_max = 0.0;
		for (ic=0;ic<gridm_nx;ic++)
		    for (jc=0;jc<gridm_ny;jc++)
			{
			kc = ic + jc * gridm_nx;
			gridm[kc] = 0.0;
			gridnm[kc] = 0;
			
			ioff = ic - (gridm_nx / 2);
			joff = jc - (gridm_ny / 2);
			istart = MAX(ioff, -ioff);
			iend = grid_nx - MAX(ioff, -ioff);
			jstart = MAX(joff, -joff);
			jend = grid_ny - MAX(joff, -joff);

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
				    gridm[kc] += (grid1[k1] - grid2[k2])
						    * (grid1[k1] - grid2[k2]);
				    gridnm[kc]++;
				    }
				}
			if (gridnm[kc] > 0)
			    {
 			    gridm[kc] = sqrt(gridm[kc]) / gridnm[kc];
			    if (misfit_max == 0.0)
			    	{
				misfit_min = gridm[kc];
				mbna_minmisfit_offset_x = (ic - gridm_nx / 2) * grid_dx;
				mbna_minmisfit_offset_y = (jc - gridm_ny / 2) * grid_dy;
				}
			    if (misfit_min > gridm[kc])
			    	{
			    	misfit_min = gridm[kc];
				mbna_minmisfit_offset_x = (ic - gridm_nx / 2) * grid_dx;
				mbna_minmisfit_offset_y = (jc - gridm_ny / 2) * grid_dy;
/*fprintf(stderr,"ic:%d jc:%d misfit:%f  pos:%f %f\n",
ic,jc,misfit_min,mbna_minmisfit_offset_x,mbna_minmisfit_offset_y); */
 			    	}
			    misfit_max = MAX(misfit_max, gridm[kc]);
/*fprintf(stderr, "misfit: i:%d j:%d n:%d b1:%f b2:%f m:%f\n",
ic, jc, gridnm[kc], grid1[kc], grid2[kc], gridm[kc]);*/
			    }
			}
		misfit_min = 0.99 * misfit_min;
		misfit_max = 1.01 * misfit_max;
		
		do_message_off();
 		}
			
 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
	    v->color = pixel_values[icolor + 1];
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
	fprintf(stderr,"justify_string: %f %s\n",height,string);

	return;
}
/*--------------------------------------------------------------------*/
void plot_string(double x, double y, double hgt, double angle, char *label)
{
	fprintf(stderr,"plot_string: %f %f %f %f %s\n",
		x,y,hgt,angle,label);

	return;
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
	double  	xscale, yscale;
	int 	ix, iy, idx, idy;
	int	boxoff, boxwid;
	static int 	ixo, iyo;
	static int 	izx1, izy1, izx2, izy2;
	static int 	pixel, ipixel;
	int	snav_1, snav_2;
	double	dmisfit;
	int	    	i, j, k;

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

	    /* clear screens for first plot */
	    if (plotmode == MBNA_PLOT_MODE_FIRST)
		{
		xg_fillrectangle(cont_xgid, 0, 0,
			    cont_borders[1], cont_borders[3],
			    pixel_values[0], XG_SOLIDLINE);
		xg_fillrectangle(corr_xgid, 0, 0,
			    corr_borders[1], corr_borders[3],
			    pixel_values[0], XG_SOLIDLINE);
		}
	
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
		    xg_drawline(cont_xgid, ixo, iyo, ix, iy, pixel_values[0], XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    }	
		}
	    ixo = (int)(mbna_plotx_scale * (swath1->pings[0].navlon + mbna_offset_x_old - mbna_plot_lon_min));
	    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (swath1->pings[0].navlat + mbna_offset_y_old - mbna_plot_lat_min));
	    for (i=1;i<swath1->npings;i++)
		{
		ix = (int)(mbna_plotx_scale * (swath1->pings[i].navlon + mbna_offset_x_old - mbna_plot_lon_min));
		iy = (int)(cont_borders[3] - mbna_ploty_scale * (swath1->pings[i].navlat + mbna_offset_y_old - mbna_plot_lat_min));
		xg_drawline(cont_xgid, ixo, iyo, ix, iy, pixel_values[0], XG_SOLIDLINE);
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
		    xg_fillrectangle(cont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[0], XG_SOLIDLINE);
		    xg_drawrectangle(cont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[0], XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    ix = (int)(mbna_plotx_scale * (section2->snav_lon[snav_2] - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (section2->snav_lat[snav_2] - mbna_plot_lat_min));
		    xg_fillrectangle(cont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[0], XG_SOLIDLINE);
		    xg_drawrectangle(cont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[0], XG_SOLIDLINE);
		    xg_drawline(cont_xgid, ixo, iyo, ix, iy, pixel_values[0], XG_SOLIDLINE);
		    }
		}



	    }
	
	    /* replot zoom box in white if moving that box */
	    if (plotmode == MBNA_PLOT_MODE_ZOOM)
		{
		xg_drawrectangle(cont_xgid,
				    MIN(izx1, izx2),
				    MIN(izy1, izy2),
				    MAX(izx1, izx2) - MIN(izx1, izx2),
				    MAX(izy1, izy2) - MIN(izy1, izy2),
				    pixel_values[0], XG_SOLIDLINE);
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
		    xg_drawline(cont_xgid, ixo, iyo, ix, iy, pixel, XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    }	
		}
	    ixo = (int)(mbna_plotx_scale * (swath1->pings[0].navlon + mbna_offset_x - mbna_plot_lon_min));
	    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (swath1->pings[0].navlat + mbna_offset_y - mbna_plot_lat_min));
	    for (i=1;i<swath1->npings;i++)
		{
		ix = (int)(mbna_plotx_scale * (swath1->pings[i].navlon + mbna_offset_x - mbna_plot_lon_min));
		iy = (int)(cont_borders[3] - mbna_ploty_scale * (swath1->pings[i].navlat + mbna_offset_y - mbna_plot_lat_min));
		xg_drawline(cont_xgid, ixo, iyo, ix, iy, pixel_values[1], XG_SOLIDLINE);
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
		    xg_drawline(cont_xgid, ixo, iyo, ix, iy, pixel, XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    }	
		}
	    ixo = (int)(mbna_plotx_scale * (swath2->pings[0].navlon - mbna_plot_lon_min));
	    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (swath2->pings[0].navlat - mbna_plot_lat_min));
	    for (i=1;i<swath2->npings;i++)
		{
		ix = (int)(mbna_plotx_scale * (swath2->pings[i].navlon - mbna_plot_lon_min));
		iy = (int)(cont_borders[3] - mbna_ploty_scale * (swath2->pings[i].navlat - mbna_plot_lat_min));
		xg_drawline(cont_xgid, ixo, iyo, ix, iy, pixel_values[1], XG_SOLIDLINE);
		ixo = ix;
		iyo = iy;
		}
		
	    /* plot tie points */
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
			}
		    else
			{
			boxoff = 3;
			boxwid = 7;
			snav_1 = tie->snav_1;
			snav_2 = tie->snav_2;
			}
		    ix = (int)(mbna_plotx_scale * (section1->snav_lon[snav_1] + mbna_offset_x - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (section1->snav_lat[snav_1] + mbna_offset_y - mbna_plot_lat_min));
		    xg_fillrectangle(cont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[2], XG_SOLIDLINE);
		    xg_drawrectangle(cont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[1], XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    ix = (int)(mbna_plotx_scale * (section2->snav_lon[snav_2] - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (section2->snav_lat[snav_2] - mbna_plot_lat_min));
		    xg_fillrectangle(cont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[2], XG_SOLIDLINE);
		    xg_drawrectangle(cont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[1], XG_SOLIDLINE);
		    xg_drawline(cont_xgid, ixo, iyo, ix, iy, pixel_values[1], XG_SOLIDLINE);
		    }
		}
	
	    /* plot zoom box if in zoom mode */
	    if (plotmode == MBNA_PLOT_MODE_ZOOMFIRST || plotmode == MBNA_PLOT_MODE_ZOOM)
		{
		xg_drawrectangle(cont_xgid,
				    MIN(mbna_zoom_x1, mbna_zoom_x2),
				    MIN(mbna_zoom_y1, mbna_zoom_y2),
				    MAX(mbna_zoom_x1, mbna_zoom_x2) - MIN(mbna_zoom_x1, mbna_zoom_x2),
				    MAX(mbna_zoom_y1, mbna_zoom_y2) - MIN(mbna_zoom_y1, mbna_zoom_y2),
				    pixel_values[1], XG_SOLIDLINE);
		izx1 = mbna_zoom_x1;
		izy1 = mbna_zoom_y1;
		izx2 = mbna_zoom_x2;
		izy2 = mbna_zoom_y2;
		}
		
	    /* plot misfit */
	    ixo = corr_borders[0] + (corr_borders[1] - corr_borders[0]) / 2;
	    iyo = corr_borders[2] + (corr_borders[3] - corr_borders[2]) / 2;
	    dmisfit = log10(misfit_max - misfit_min)/79.99;
	    for (i=0;i<gridm_nx;i++)
		for(j=0;j<gridm_ny;j++)
		    {
		    k = i + j * gridm_nx;
		    if (gridnm[k] > 0)
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
			ipixel = 7 + log10(gridm[k] - misfit_min)/dmisfit;
			if (ipixel < 6) ipixel = 6;
			else if (ipixel > 85) ipixel = 85;
    /*fprintf(stderr, "%d %d %f %f %f   %f %d\n",
    i, j, misfit_min, misfit_max, dmisfit, gridm[k], ipixel);  */
			xg_fillrectangle(corr_xgid,
				    ix, iy, idx, idy,
				    pixel_values[ipixel], XG_SOLIDLINE);
			}
		    }
		
	    /* draw boundary and crosshair */
	    xg_drawline(corr_xgid,
			    ixo - (int)(mbna_misfit_scale * mbna_misfit_offset_x), 
			    corr_borders[2],
			    ixo - (int)(mbna_misfit_scale * mbna_misfit_offset_x), 
			    corr_borders[3],
			    pixel_values[1], XG_SOLIDLINE);
	    xg_drawline(corr_xgid,
			    corr_borders[0], 
			    iyo + (int)(mbna_misfit_scale * mbna_misfit_offset_y),
			    corr_borders[1], 
			    iyo + (int)(mbna_misfit_scale * mbna_misfit_offset_y),
			    pixel_values[1], XG_SOLIDLINE);
	
	    /* draw working offset */
	    ix = ixo + (int)(mbna_misfit_scale * (mbna_offset_x - mbna_misfit_offset_x));
	    iy = iyo - (int)(mbna_misfit_scale * (mbna_offset_y - mbna_misfit_offset_y));
	    xg_fillrectangle(corr_xgid, ix-3, iy-3, 7, 7, pixel_values[2], XG_SOLIDLINE);
	    xg_drawrectangle(corr_xgid, ix-3, iy-3, 7, 7, pixel_values[1], XG_SOLIDLINE);
	
	    /* draw x at minimum misfit */
	    ix = ixo + (int)(mbna_misfit_scale * mbna_minmisfit_offset_x);
	    iy = iyo - (int)(mbna_misfit_scale * mbna_minmisfit_offset_y);
	    xg_drawline(corr_xgid,
			    ix - 10, iy + 10,
			    ix + 10, iy - 10,
			    pixel_values[1], XG_SOLIDLINE);
	    xg_drawline(corr_xgid,
			    ix + 10, iy + 10,
			    ix - 10, iy - 10,
			    pixel_values[1], XG_SOLIDLINE);
	
	    /* draw dashed + at inversion solution */
	    if (project.inversion != MBNA_INVERSION_NONE)
	    	{
	    	ix = ixo + (int)(mbna_misfit_scale * (mbna_invert_offset_x - mbna_misfit_offset_x));
	    	iy = iyo - (int)(mbna_misfit_scale * (mbna_invert_offset_y - mbna_misfit_offset_y));
	    	xg_drawline(corr_xgid,
			    ix - 10, iy,
			    ix + 10, iy,
			    pixel_values[3], XG_SOLIDLINE);
	    	xg_drawline(corr_xgid,
			    ix, iy + 10,
			    ix, iy - 10,
			    pixel_values[3], XG_SOLIDLINE);
	    	xg_drawline(corr_xgid,
			    ix - 10, iy,
			    ix + 10, iy,
			    pixel_values[1], XG_SOLIDLINE);
	    	xg_drawline(corr_xgid,
			    ix, iy + 10,
			    ix, iy - 10,
			    pixel_values[1], XG_SOLIDLINE);
		}
	    }
			
 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}
}
/*--------------------------------------------------------------------*/

int
mbnavadjust_invertnav()
{
	/* local variables */
	char	*function_name = "mbnavadjust_invertnav";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	nnav, nsnav, nfix, ndx, ndx2, ncrossing, nconstraint;
	int	nseq;
	int	nnz = 3;
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
	double	smoothweight_first, sigma_total_first, sigma_crossing_first;
	double	smoothweight_old, smoothweight_best, sigma_total_best, sigma_crossing_best;
	double	smoothfactor, smoothmin, smoothmax, sigmamin, sigmamax;
	double	offset_x, offset_y;
	int	first;
	int	iter;
	char	npath[STRING_MAX];
	char	apath[STRING_MAX];
	char	opath[STRING_MAX];
	FILE	*nfp, *afp, *ofp;
	char	*result;
	char	buffer[BUFFER_MAX];
	int	done, nscan;
	double	factor;
	int	mbp_heading_mode;
	double	mbp_headingbias;
	int	mbp_rollbias_mode;
	double	mbp_rollbias;
	double	mbp_rollbias_port;
	double	mbp_rollbias_stbd;
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
		&& project.num_crossings_analyzed == project.num_crossings)
    		{
     		/* retrieve crossing parameters */
		/* count up the unknowns and constraints to get size of
		 * inverse problem:
		 *	    nconstraint = 2 * nsnav + ncrossing
		 */
		/* set message dialog on */
		sprintf(message,"Setting up navigation inversion...");
		do_message_on(message);

		nnav = 0;
		nsnav = 0;
		nfix = 0;
		ncrossing = 0;
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
				
				/* add first derivative constraint if nseq > 1 */
				if (nseq > 1)
				    {
				    avg_dtime_d += section->snav_time_d[isnav] - time_d_old;
				    ndx++;
				    }
				
				/* add second derivative constraint if nseq > 2 */
				if (nseq > 2)
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
			ncrossing += crossing->num_ties;
			for (j=0;j<crossing->num_ties;j++)
			    {
			    avg_offset += fabs(crossing->ties[j].offset_x);
			    }
			}
		    }
		if (ncrossing > 0)
		    avg_offset /= ncrossing;
		
		/* allocate space for the inverse problem */
		nconstraint = 2 * (nfix + ndx + ndx2 + ncrossing);
		nrows = nconstraint;
		ncols = 2 * nsnav;
		status = mb_malloc(mbna_verbose, nnz * nrows * sizeof(double), &a,&error);
		status = mb_malloc(mbna_verbose, nnz * nrows * sizeof(int), &ia,&error);
		status = mb_malloc(mbna_verbose, nrows * sizeof(int), &nia,&error);
		status = mb_malloc(mbna_verbose, nrows * sizeof(double), &d,&error);
		status = mb_malloc(mbna_verbose, ncols * sizeof(double), &x,&error);
		status = mb_malloc(mbna_verbose, ncols * sizeof(int), &nx,&error);
		status = mb_malloc(mbna_verbose, ncols * sizeof(double), &dx,&error);
		status = mb_malloc(mbna_verbose, ncycle * sizeof(double), &sigma,&error);
		status = mb_malloc(mbna_verbose, ncycle * sizeof(double), &work,&error);

		/* if error initializing memory then don't invert */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(mbna_verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			}
		}
		
	/* proceed with construction of inverse problems */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& project.num_crossings_analyzed == project.num_crossings
		&& error == MB_ERROR_NO_ERROR)
    		{
		/* add info text */
		sprintf(message, "Inverting for optimal navigation\n");
		do_info_add(message, MB_YES);
		sprintf(message, " > Inverse problem size:\n");
		do_info_add(message, MB_NO);
		sprintf(message, " >   Nav points:                    %d\n", nnav);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Independent nav snav points:    %d\n", nsnav);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Fixed nav snav points:          %d\n", nfix);
		do_info_add(message, MB_NO);
		sprintf(message, " >   First derivative constraints:  %d\n", ndx);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Second derivative constraints: %d\n", ndx2);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Useful crossings:              %d\n", ncrossing);
		do_info_add(message, MB_NO);
		sprintf(message, " >   Total Unknowns:                %d\n", 2 * nsnav);
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
		for (i=0;i<2*nsnav;i++)
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
				    }
				
				/* add first derivative constraint if nseq > 1 */
				if (nseq > 1)
				    {
				    dtime_d = section->snav_time_d[isnav] - time_d_old;
				
				    /* longitude component */
				    k = nnz * nr;
				    a[k] = -smoothweight_old / dtime_d;
				    a[k+1] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 2;
				    ia[k+1] = nc;
				    nia[nr] = 2;
				    nr++;
				
				    /* latitude component */
				    k = nnz * nr;
				    a[k] = -smoothweight_old / dtime_d;
				    a[k+1] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 1;
				    ia[k+1] = nc + 1;
				    nia[nr] = 2;
				    nr++;
				    }
				
				/* add second derivative constraint if nseq > 2 */
				if (nseq > 2)
				    {
				    dtime_d = section->snav_time_d[isnav] - time_d_older;
				
				    /* longitude component */
				    k = nnz * nr;
				    a[k] = smoothweight_old / dtime_d;
				    a[k+1] = -2.0 * smoothweight_old / dtime_d;
				    a[k+2] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 4;
				    ia[k+1] = nc - 2;
				    ia[k+2] = nc;
				    nia[nr] = 3;
				    nr++;
				
				    /* latitude component */
				    k = nnz * nr;
				    a[k] = smoothweight_old / dtime_d;
				    a[k+1] = -2.0 * smoothweight_old / dtime_d;
				    a[k+2] = smoothweight_old / dtime_d;
				    d[nr] = 0.0;
				    ia[k] = nc - 3;
				    ia[k+1] = nc - 1;
				    ia[k+2] = nc + 1;
				    nia[nr] = 3;
				    nr++;
				    }
				
				/* save time_d values */
				time_d_older = time_d_old;
				time_d_old = section->snav_time_d[isnav];

				/* add to unknown count */
				nc += 2;
				}
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
			nc1 = section->global_start_snav + tie->snav_1;

			/* get absolute id for second snav point */
			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];
			nc2 = section->global_start_snav + tie->snav_2;
if (nc1 > nsnav - 1 || nc2 > nsnav -1
|| nc1 < 0 || nc2 < 0)
fprintf(stderr, "BAD snav ID: %d %d %d\n", nc1, nc2, nsnav);
			
			/* make longitude constraint */
			k = nnz * nr;
			a[k] = -mbna_offsetweight;
			a[k+1] = mbna_offsetweight;
			d[nr] = mbna_offsetweight * tie->offset_x;
			ia[k] = 2 * nc1;
			ia[k+1] = 2 * nc2;
			nia[nr] = 2;
			nx[2 * nc1]++;
			nx[2 * nc2]++;
			nr++;
			
			/* make latitude constraint */
			k = nnz * nr;
			a[k] = -mbna_offsetweight;
			a[k+1] = mbna_offsetweight;
			d[nr] = mbna_offsetweight * tie->offset_y;
			ia[k] = 2 * nc1 + 1;
			ia[k+1] = 2 * nc2 + 1;
			nia[nr] = 2;
			nx[2 * nc1 + 1]++;
			nx[2 * nc2 + 1]++;
			nr++;
			}
		    }

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
		    for (i=0;i<2*(nfix+ndx+ndx2);i++)
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
			sprintf(message,"Inverting %dx%d: iter:%d smooth:%.2g ratio:%.3f",
				nc, nr, iter, smoothweight,
				(sigma_crossing / sigma_crossing_first));
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
		    if (mbna_verbose > 1)
		    fprintf(stderr, "Initial lspeig: %g %g %g %g\n",
			sup, smax, err, supt);
		    ncyc = 16;
		    for (i=0;i<4;i++)
			{
			lspeig(a, ia, nia, nnz, nc, nr, ncyc,
				&nsig, x, dx, sigma, work, &smax, &err, &sup);
			supt = smax + err;
			if (sup > supt)
			    supt = sup;
			if (mbna_verbose > 1)
			fprintf(stderr, "lspeig[%d]: %g %g %g %g\n",
			    i, sup, smax, err, supt);
			}
			
		    /* calculate chebyshev factors (errlsq is the theoretical error) */
		    slo = supt / bandwidth;
		    chebyu(sigma, ncycle, supt, slo, work);
		    errlsq = errlim(sigma, ncycle, supt, slo);
		    if (mbna_verbose > 1)
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
		    for (i=0;i<nc/2;i++)
			{
			fprintf(stderr, "i:%d  offsets: %f %f  crossings: %d %d\n",
				i, x[2*i], x[2*i+1], nx[2*i], nx[2*i+1]);
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
			if (i >= (nr - 2 * ncrossing))
			    sigma_crossing += (d[i] - s) * (d[i] - s);
			}
		    sigma_total = sqrt(sigma_total) / nr;
		    sigma_crossing = sqrt(sigma_crossing) / ncrossing;
		
		    /* keep track of results */
		    if (first == MB_YES)
			{
			first = MB_NO;
			smoothweight_first = smoothweight;
			sigma_total_first = sigma_total;
			sigma_crossing_first = MAX(sigma_crossing,1e-5);
			smoothweight_old = smoothweight;
			smoothmin = smoothweight;
			sigmamin = sigma_crossing;
			}
		    else if (sigma_crossing >= 1.005 * sigma_crossing_first
			    && sigma_crossing <= 1.01 * sigma_crossing_first
			    && sigma_crossing > 0.0000001)
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
			    sigmamin = sigma_crossing;
			    }
			if (smoothmax > 0.0)
			    smoothfactor = (smoothmin + 0.3 * (smoothmax - smoothmin))
						/ smoothweight;				
			smoothweight_old = smoothweight;
			}
		    else if (sigma_crossing > 1.01 * sigma_crossing_first
			    && sigma_crossing > 0.0000001)
			{
			if (smoothweight < smoothmax || smoothmax < 0.0)
			    {
			    smoothmax = smoothweight;
			    sigmamax = sigma_crossing;
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
			    iter, smoothweight, sigma_total, sigma_crossing, 
			    (sigma_crossing / sigma_crossing_first));
		    do_info_add(message, MB_NO);
fprintf(stderr,"iteration:%3d smooth:%12g sigmatot:%12g sigmacrossing:%12g ratio:%12g\n",
iter,smoothweight,sigma_total,sigma_crossing,
(sigma_crossing / sigma_crossing_first));
		    }

		/* save solution */
		k = -2;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    if (isnav > 0 || section->continuity == MB_NO)
				k += 2;
			    section->snav_lon_offset[isnav] = x[k];
			    section->snav_lat_offset[isnav] = x[k+1];
			    }
			}
		    }
		}
		
	/* output results from navigation solution */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& project.num_crossings_analyzed == project.num_crossings
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
			tie->inversion_status = MBNA_INVERSION_CURRENT;
    			tie->inversion_offset_x = offset_x;
    			tie->inversion_offset_y = offset_y;
    			tie->inversion_offset_x_m = offset_x / mbna_mtodeglon;
    			tie->inversion_offset_y_m = offset_y / mbna_mtodeglat;

			sprintf(message, " >     %4d   %10.3f %10.3f   %10.3f %10.3f   %10.3f %10.3f\n",
				icrossing,
				tie->offset_x / mbna_mtodeglon,
				tie->offset_y / mbna_mtodeglat,
				offset_x / mbna_mtodeglon,
				offset_y / mbna_mtodeglat,
				(offset_x - tie->offset_x) / mbna_mtodeglon,
				(offset_y - tie->offset_y) / mbna_mtodeglat);
			do_info_add(message, MB_NO);
			}
		    }

		if (mbna_verbose >= 0)
		for (i=0;i<nc/2;i++)
		    {
		    if (mbna_verbose > 1)
		    fprintf(stderr, "i:%d  offsets: %f %f  crossings: %d %d\n",
		    		i, x[2*i], x[2*i+1], nx[2*i], nx[2*i+1]);
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
		status = mb_free(mbna_verbose, &a,&error);
		status = mb_free(mbna_verbose, &ia,&error);
		status = mb_free(mbna_verbose, &nia,&error);
		status = mb_free(mbna_verbose, &d,&error);
		status = mb_free(mbna_verbose, &x,&error);
		status = mb_free(mbna_verbose, &nx,&error);
		status = mb_free(mbna_verbose, &dx,&error);
		status = mb_free(mbna_verbose, &sigma,&error);
		status = mb_free(mbna_verbose, &work,&error);

		/* generate new nav files */
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    if (file->status != MBNA_FILE_FIXED)
		    	{
		    	time_d_older = section->snav_time_d[0];
		    	sprintf(npath,"%s/nvs_%4.4d.mb166",
			    project.datadir,i);
		    	sprintf(apath,"%s/nvs_%4.4d.na%d",
			    project.datadir,i,file->output_id);
		    	sprintf(opath,"%s.na%d", file->file, file->output_id);
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
				    navlon -= section->snav_lon_offset[isnav]
						    + factor * (section->snav_lon_offset[isnav+1]
							    - section->snav_lon_offset[isnav]);
				    navlat -= section->snav_lat_offset[isnav]
						    + factor * (section->snav_lat_offset[isnav+1]
							    - section->snav_lat_offset[isnav]);
				    
				    /* write the updated nav out */
				    fprintf(ofp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						time_i[0], time_i[1], time_i[2], time_i[3],
						time_i[4], time_i[5], time_i[6], time_d,
						navlon, navlat, heading, speed, 
						draft, roll, pitch, heave);
				    fprintf(afp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						time_i[0], time_i[1], time_i[2], time_i[3],
						time_i[4], time_i[5], time_i[6], time_d,
						navlon, navlat, heading, speed, 
						draft, roll, pitch, heave);
/*fprintf(stderr, "%2.2d:%2.2d:%2.2d:%5.3f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f\n",
i, isection, isnav, factor, 
time_i[0], time_i[1], time_i[2], time_i[3],
time_i[4], time_i[5], time_i[6], time_d,
navlon, navlat, heading, speed);*/
				    }
			    	}
			    }
		    	if (nfp != NULL) fclose(nfp);
		    	if (afp != NULL) fclose(afp);
		    	if (ofp != NULL) 
			    {
			    fclose(ofp);
			    
			    /* get bias values */
			    mb_pr_get_heading(mbna_verbose, file->file, 
						&mbp_heading_mode, 
						&mbp_headingbias, 
						error);
			    mb_pr_get_rollbias(mbna_verbose, file->file, 
						&mbp_rollbias_mode, 
						&mbp_rollbias, 
						&mbp_rollbias_port, 
						&mbp_rollbias_stbd, 
						error);
	    
			    /* update output file in mbprocess parameter file */
			    status = mb_pr_update_navadj(mbna_verbose, file->file, 
					MBP_NAV_ON, opath, 
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
			    status = mb_pr_update_heading(mbna_verbose, file->file, 
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
			    status = mb_pr_update_rollbias(mbna_verbose, file->file, 
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
