/*--------------------------------------------------------------------
 *    The MB-system:	hsvelocitytool.c	3.00	6/6/93
 *    $Id: hsvelocitytool.c,v 1.1 1993-08-16 23:28:30 caress Exp $
 *
 *    Copyright (c) 1993 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * HSVELOCITYTOOL is an interactive water velocity profile editor
 * used to examine multiple water velocity profiles and to create
 * new water velocity profiles which can be used for the processing
 * of hydrosweep multibeam sonar data.  In general, this tool is used to examine
 * water velocity profiles obtained from XBTs, CTDs, or databases,
 * and to construct new profiles consistent with these various
 * sources of information.
 *
 * Author:	D. W. Caress
 * Date:	June 6, 1993
 *
 * $Log: not supported by cvs2svn $
 *
 */

/*--------------------------------------------------------------------*/

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "../../include/mb_format.h"
#include "../../include/mb_status.h"
#include "../../include/mb_io.h"
#include "../../include/mbsys_hsds.h"

/* xgraphics defines */
#define	CLEAR_ALL	0
#define	BLACK_ALL	1
#define	OVERLAY1_CLEAR	64
#define OVERLAY1_DRAW	65
#define OVERLAY1_DASH	66
#define OVERLAY2_CLEAR	128
#define OVERLAY2_DRAW	129
#define OVERLAY2_DASH	130
#define	WHITE	
#define	BLACK	

/* DTR define */
#define DTR (M_PI/180.)

/* velocity profile structure definition */
struct profile
	{
	int	n;
	char	name[128];
	double	*depth;
	double	*velocity;
	double	*velocity_layer;
	};

/* id variables */
static char rcs_id[] = "$Id: hsvelocitytool.c,v 1.1 1993-08-16 23:28:30 caress Exp $";
static char program_name[] = "HSVELOCITYTOOL";
static char help_message[] = "HSVELOCITYTOOL is an interactive water velocity profile editor  \nused to examine multiple water velocity profiles and to create  \nnew water velocity profiles which can be used for the processing  \nof hydrosweep multibeam sonar data.  In general, this tool is used to  \nexamine water velocity profiles obtained from XBTs, CTDs, or  \ndatabases, and to construct new profiles consistent with these  \nvarious sources of information.";
static char usage_message[] = "hsvelocitytool [-V -H]";

/* status variables */
int	error = MB_ERROR_NO_ERROR;
int	verbose = 0;
char	*message;

/* hsvelocitytool control variables */
#define	MAX_PROFILES	10
#define	PICK_DISTANCE	50
struct profile	profile_display[MAX_PROFILES];
struct profile	profile_edit;
int	*edit_x;
int	*edit_y;
char	editfile[128];
int	edit = 0;
int	ndisplay = 0;
int	hsvt_xgid;
int	borders[4];
int	maxdepth = 3000;
int	velrange = 500;
int	resrange = 10;

/* plotting variables */
int	xmin, xmax, ymin, ymax;
double	xminimum, xmaximum, yminimum, ymaximum;
double	xscale, yscale;
int	xrmin, xrmax, yrmin, yrmax;
double	xrminimum, xrmaximum, yrminimum, yrmaximum;
double	xrscale, yrscale;
int	active = -1;

/* default edit profile */
int	numedit = 14;
int	depthedit[14] = {   0, 100, 200, 400, 800, 1200, 1600, 2000,
			3000, 4000, 5000, 7000, 9000, 12000};
int	veledit[28] = { 1500, 1500, 1500, 1500, 1500,
			1500, 1500, 1500, 1500, 1500,
			1500, 1500, 1500, 1500 };

/* MBIO control parameters */
int	format;
int	pings;
int	lonflip;
double	bounds[4];
int	btime_i[6];
int	etime_i[6];
double	btime_d;
double	etime_d;
double	speedmin;
double	timegap;
int	beams_bath;
int	beams_back;
char	*mbio_ptr;

/* mbio read and write values */
char	*store_ptr;
int	kind;
int	id;
int	time_i[6];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
double	distance;
int	nbath;
int	*bath;
int	*bathdist;
int	nback;
int	*back;
int	*backdist;
char	comment[256];

/* buffer control variables */
#define	HSVT_BUFFER_SIZE	1000
char	*buff_ptr;
int	buffer_size = HSVT_BUFFER_SIZE;
int	nbuffer;
int	nload;

/* survey ping raytracing arrays */
double	dangle;
double	s_angle[MBSYS_HSDS_BEAMS];
double	s_p[MBSYS_HSDS_BEAMS];
double	*s_ttime_tab[MBSYS_HSDS_BEAMS];
double	*s_dist_tab[MBSYS_HSDS_BEAMS];

/* depth range variables */
double	bath_min;
double	bath_max;

/* residual variables */
double	residual[MBSYS_HSDS_BEAMS];
int	nresidual[MBSYS_HSDS_BEAMS];

/*--------------------------------------------------------------------*/
int hsvt_init(argc,argv)
int	argc;
char	**argv;
{
	/* local variables */
	char	*function_name = "hsvt_init";
	int	status = MB_SUCCESS;
	int	fileflag = 0;
	int	i;

	/* parsing variables */
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* set default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
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
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;
	dangle = MBSYS_HSDS_BEAM_SPACING;

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:VvHh")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%lf", &dangle);
			flag++;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
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
		exit(MB_FAILURE);
		}

	/* print starting message */
	if (verbose == 1)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:            %d\n",verbose);
		fprintf(stderr,"dbg2       help:               %d\n",help);
		fprintf(stderr,"dbg2       beam angle spacing: %f\n",dangle);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
		}

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       argc:      %d\n",argc);
		for (i=0;i<argc;i++)
			fprintf(stderr,"dbg2       argv[%d]:    %s\n",
				i,argv[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
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
int hsvt_set_graphics(xgid,brdr)
int	xgid;
int	*brdr;
{
	/* local variables */
	char	*function_name = "hsvt_set_graphics";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       xgid:         %d\n",xgid);
		for (i=0;i<4;i++)
			fprintf(stderr,"dbg2       borders[%d]:   %d\n",
				i,brdr[0]);
		}

	/* set graphics id */
	hsvt_xgid = xgid;

	/* set graphics bounds */
	for (i=0;i<4;i++)
		borders[i] = brdr[i];

	/* print output debug statements */
	if (verbose >= 2)
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
int hsvt_get_defaults(s_edit,s_ndisplay,s_maxdepth,
	s_velrange,s_resrange,s_format,s_nbuffer)
int	*s_edit;
int	*s_ndisplay;
int	*s_maxdepth;
int	*s_velrange;
int	*s_resrange;
int	*s_format;
int	*s_nbuffer;
{
	/* local variables */
	char	*function_name = "hsvt_get_defaults";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* set values */
	*s_edit = edit;
	*s_ndisplay = ndisplay;
	*s_maxdepth = maxdepth;
	*s_velrange = velrange;
	*s_resrange = resrange;
	*s_format = format;
	*s_nbuffer = nbuffer ;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       s_edit:      %d\n",*s_edit);
		fprintf(stderr,"dbg2       s_ndisplay:  %d\n",*s_ndisplay);
		fprintf(stderr,"dbg2       s_maxdepth:  %d\n",*s_maxdepth);
		fprintf(stderr,"dbg2       s_velrange:  %d\n",*s_velrange);
		fprintf(stderr,"dbg2       s_resrange:  %d\n",*s_resrange);
		fprintf(stderr,"dbg2       s_format:    %d\n",*s_format);
		fprintf(stderr,"dbg2       s_nbuffer:   %d\n",*s_nbuffer);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_set_values(s_edit,s_ndisplay,s_maxdepth,s_velrange,s_resrange)
int	s_edit;
int	s_ndisplay;
int	s_maxdepth;
int	s_velrange;
int	s_resrange;
{
	/* local variables */
	char	*function_name = "hsvt_set_values";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input values:\n");
		fprintf(stderr,"dbg2       s_edit:      %d\n",s_edit);
		fprintf(stderr,"dbg2       s_ndisplay:  %d\n",s_ndisplay);
		fprintf(stderr,"dbg2       s_maxdepth:  %d\n",s_maxdepth);
		fprintf(stderr,"dbg2       s_velrange:  %d\n",s_velrange);
		fprintf(stderr,"dbg2       s_resrange:  %d\n",s_resrange);
		}

	/* set values */
	edit = s_edit;
	ndisplay = s_ndisplay;
	maxdepth = s_maxdepth;
	velrange = s_velrange;
	resrange = s_resrange;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_open_edit_profile(file)
char	*file;
{
	/* local variables */
	char	*function_name = "hsvt_open_edit_profile";
	int	status = MB_SUCCESS;
	int	size;
	char	buffer[128];
	char	*result;
	struct profile *profile;
	FILE	*fp;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input values:\n");
		fprintf(stderr,"dbg2       file:        %s\n",file);
		}

	/* get profile pointer */
	profile = &profile_edit;

	/* clear out old velocity data */
	if (edit == MB_YES)
		{
		edit = MB_NO;
		profile->n = 0;
		strcpy(profile->name,"\0");
		mb_free(verbose,edit_x,&error);
		mb_free(verbose,edit_y,&error);
		mb_free(verbose,profile->depth,&error);
		mb_free(verbose,profile->velocity,&error);
		mb_free(verbose,profile->velocity_layer,&error);
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			mb_free(verbose,s_ttime_tab[i],&error);
			mb_free(verbose,s_dist_tab[i],&error);
			}
		}

	/* open the file if possible and count the velocity points */
	profile->n = 0;
	if ((fp = fopen(file, "r")) == NULL) 
		{
		status = MB_FAILURE;
		fprintf(stderr,"\nUnable to Open Velocity Profile File <%s> for reading\n",file);
		return(status);
		}
	while ((result = fgets(buffer,128,fp)) == buffer)
		if (buffer[0] != '#')
			profile->n++;
	fclose(fp);

	/* allocate space for the velocity profile and raytracing tables */
	size = profile->n*sizeof(int);
	status = mb_malloc(verbose,size,&edit_x,&error);
	status = mb_malloc(verbose,size,&edit_y,&error);
	size = profile->n*sizeof(double);
	status = mb_malloc(verbose,size,&(profile->depth),&error);
	status = mb_malloc(verbose,size,&(profile->velocity),&error);
	status = mb_malloc(verbose,size,&(profile->velocity_layer),&error);
	for (i=0;i<MBSYS_HSDS_BEAMS;i++)
		{
		status = mb_malloc(verbose,size,&s_ttime_tab[i],&error);
		status = mb_malloc(verbose,size,&s_dist_tab[i],&error);
		}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* open the file if possible and read the velocity points */
	profile->n = 0;
	strcpy(profile->name,file);
	if ((fp = fopen(file, "r")) == NULL) 
		{
		status = MB_FAILURE;
		fprintf(stderr,"\nUnable to Open Velocity Profile File <%s> for reading\n",file);
		return(status);
		}
	strncpy(buffer,"\0",sizeof(buffer));
	while ((result = fgets(buffer,128,fp)) == buffer)
		{
		if (buffer[0] != '#')
			{
			sscanf(buffer,"%lf %lf",
				&(profile->depth[profile->n]),
				&(profile->velocity[profile->n]));

			/* output some debug values */
			if (verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  New velocity value read in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       dep[%d]: %f  vel[%d]: %f\n",
					profile->n,
					profile->depth[profile->n],
					profile->n,
					profile->velocity[profile->n]);
				}
			profile->n++;
			}
		strncpy(buffer,"\0",sizeof(buffer));
		}
	fclose(fp);

	/* assume success */
	edit = MB_YES;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_new_edit_profile()
{
	/* local variables */
	char	*function_name = "hsvt_new_edit_profile";
	int	status = MB_SUCCESS;
	struct profile *profile;
	int	size;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* get profile pointer */
	profile = &profile_edit;

	/* clear out old velocity data */
	if (edit == MB_YES)
		{
		edit = MB_NO;
		profile->n = 0;
		strcpy(profile->name,"\0");
		mb_free(verbose,edit_x,&error);
		mb_free(verbose,edit_y,&error);
		mb_free(verbose,profile->depth,&error);
		mb_free(verbose,profile->velocity,&error);
		mb_free(verbose,profile->velocity_layer,&error);
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			mb_free(verbose,s_ttime_tab[i],&error);
			mb_free(verbose,s_dist_tab[i],&error);
			}
		}

	/* allocate space for the velocity profile and raytracing tables */
	profile->n = numedit;
	size = profile->n*sizeof(int);
	status = mb_malloc(verbose,size,&edit_x,&error);
	status = mb_malloc(verbose,size,&edit_y,&error);
	size = profile->n*sizeof(double);
	status = mb_malloc(verbose,size,&(profile->depth),&error);
	status = mb_malloc(verbose,size,&(profile->velocity),&error);
	status = mb_malloc(verbose,size,&(profile->velocity_layer),&error);
	for (i=0;i<MBSYS_HSDS_BEAMS;i++)
		{
		status = mb_malloc(verbose,size,&s_ttime_tab[i],&error);
		status = mb_malloc(verbose,size,&s_dist_tab[i],&error);
		}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* copy the default values */
	strcpy(profile->name,"new");
	for (i=0;i<profile->n;i++)
		{
		profile->depth[i] = depthedit[i];
		profile->velocity[i] = veledit[i];
		}

	/* assume success */
	edit = MB_YES;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_save_edit_profile(file)
char	*file;
{
	/* local variables */
	char	*function_name = "hsvt_save_edit_profile";
	int	status = MB_SUCCESS;
	struct profile *profile;
	FILE	*fp;
	int	i;

	/* time, user, host variables */
	long int	right_now;
	char	date[25], user[128], host[128];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input values:\n");
		fprintf(stderr,"dbg2       file:        %s\n",file);
		}

	/* get profile pointer */
	profile = &profile_edit;

	/* open the file if possible */
	if ((fp = fopen(file, "w")) == NULL) 
		{
		status = MB_FAILURE;
		fprintf(stderr,"\nUnable to Open Velocity Profile File <%s> for writing\n",file);
		return(status);
		}

	/* write comments */
	fprintf(fp,"# Water velocity profile created by program %s\n",
		program_name);
	fprintf(fp,"# Version %s\n",rcs_id);
	right_now = time((long *)0);
	strncpy(date,"\0",25);
	right_now = time((long *)0);
	strncpy(date,ctime(&right_now),24);
	strcpy(user,getenv("USER"));
	gethostname(host,128);
	fprintf(fp,"# Run by user <%s> on cpu <%s> at <%s>\n",
		user,host,date);

	/* write the profile */
	for (i=0;i<profile->n;i++)
		fprintf(fp,"%f %f\n",profile->depth[i],
			profile->velocity[i]);

	/* close the file */
	fclose(fp);

	/* assume success */
	edit = MB_YES;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_open_display_profile(file)
char	*file;
{
	/* local variables */
	char	*function_name = "hsvt_open_display_profile";
	int	status = MB_SUCCESS;
	char	buffer[128];
	char	*result;
	struct profile *profile;
	FILE	*fp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input values:\n");
		fprintf(stderr,"dbg2       file:        %s\n",file);
		}

	/* check that there is room for this data */
	if (ndisplay >= MAX_PROFILES)
		{
		status = MB_FAILURE;
		fprintf(stderr,"\nNo room for another display velocity profile\n");
		return(status);
		}

	/* get profile pointer */
	profile = &profile_display[ndisplay];

	/* open the file if possible and count the velocity points */
	profile->n = 0;
	if ((fp = fopen(file, "r")) == NULL) 
		{
		status = MB_FAILURE;
		fprintf(stderr,"\nUnable to Open Velocity Profile File <%s> for reading\n",file);
		return(status);
		}
	while ((result = fgets(buffer,128,fp)) == buffer)
		if (buffer[0] != '#')
			profile->n++;
	fclose(fp);

	/* allocate space for the velocity profile and raytracing tables */
	status = mb_malloc(verbose,profile->n*sizeof(double),
		&(profile->depth),&error);
	status = mb_malloc(verbose,profile->n*sizeof(double),
		&(profile->velocity),&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* open the file if possible and read the velocity points */
	profile->n = 0;
	strcpy(profile->name,file);
	if ((fp = fopen(file, "r")) == NULL) 
		{
		status = MB_FAILURE;
		fprintf(stderr,"\nUnable to Open Velocity Profile File <%s> for reading\n",file);
		return(status);
		}
	strncpy(buffer,"\0",sizeof(buffer));
	while ((result = fgets(buffer,128,fp)) == buffer)
		{
		if (buffer[0] != '#')
			{
			sscanf(buffer,"%lf %lf",
				&(profile->depth[profile->n]),
				&(profile->velocity[profile->n]));

			/* output some debug values */
			if (verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  New velocity value read in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       dep[%d]: %f  vel[%d]: %f\n",
					profile->n,
					profile->depth[profile->n],
					profile->n,
					profile->velocity[profile->n]);
				}
			profile->n++;
			}
		strncpy(buffer,"\0",sizeof(buffer));
		}
	fclose(fp);

	/* assume success */
	ndisplay++;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_get_display_names(nlist,list)
int	*nlist;
char	*list[MAX_PROFILES];
{
	/* local variables */
	char	*function_name = "hsvt_get_display_names";
	char	*string;
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input values:\n");
		fprintf(stderr,"dbg2       list:        %d\n",list);
		}

	/* set values */
	*nlist = ndisplay;
	for (i=0;i<*nlist;i++)
		list[i] = profile_display[i].name;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nlist:       %d\n",*nlist);
		for (i=0;i<*nlist;i++)
			fprintf(stderr,"dbg2       name[%d]: %s\n",i,list[i]);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_delete_display_profile(select)
int	select;
{
	/* local variables */
	char	*function_name = "hsvt_delete_display_profile";
	struct profile *profile;
	int	status = MB_SUCCESS;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input values:\n");
		fprintf(stderr,"dbg2       select:      %d\n",select);
		}

	/* check if reasonable */
	if (select < ndisplay)
		{
		/* remove selected profile */
		profile = &profile_display[select];
		profile->n = 0;
		strcpy(profile->name,"\0");
		mb_free(verbose,profile->depth,&error);
		mb_free(verbose,profile->velocity,&error);

		/* reorder remaining profiles */
		for (i=select+1;i<ndisplay;i++)
			{
			profile_display[i-1].n = profile_display[i].n;
			strcpy(profile_display[i-1].name,
				profile_display[i].name);
			for (j=0;j<profile_display[i-1].n;j++)
				{
				profile_display[i-1].depth[j] 
					= profile_display[i].depth[j];
				profile_display[i-1].velocity[j] 
					= profile_display[i].velocity[j];
				}
			}
		ndisplay--;
		}
	else
		status = MB_FAILURE;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_plot()
{
	/* local variables */
	char	*function_name = "hsvt_plot";
	struct profile *profile;
	int	status = MB_SUCCESS;

	/* plotting variables */
	int	margin;
	int	xcen, ycen;
	double	deltax, deltay;
	int	nx_int, ny_int;
	int	x_int, y_int;
	int	xrcen, yrcen;
	double	deltaxr, deltayr;
	int	nxr_int, nyr_int;
	int	xr_int, yr_int;
	int	xx, vx, yy, vy;
	int	xxo, yyo;
	int	swidth, sascent, sdescent;
	char	string[128];
	int	color;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* turn clipp mask back to whole canvas */
	xg_setclip(hsvt_xgid,borders[0],borders[2],
		borders[1]-borders[0],borders[3]-borders[2]);

	/* clear screen */
	xg_fillrectangle(hsvt_xgid,borders[0],borders[2],
		borders[1]-borders[0],borders[3]-borders[2],CLEAR_ALL);

	/* set scaling */
	margin = (borders[1] - borders[0])/15;
	xmin = 2*margin;
	xmax = borders[1] - margin;
	ymin = margin;
	ymax = borders[3] - 6*margin;
	xcen = xmin + (xmax - xmin)/2;
	ycen = ymin + (ymax - ymin)/2;
	xminimum = 1490.0 - velrange/2;
	xmaximum = 1490.0 + velrange/2;
	deltax = 0.1*(xmaximum - xminimum);
	xscale = (xmax - xmin)/(xmaximum - xminimum);
	x_int = deltax*xscale;
	nx_int = (xmaximum - xminimum)/deltax + 1;
	yminimum = 0.0;
	ymaximum = maxdepth;
	deltay = 0.1*(ymaximum - yminimum);
	yscale = (ymax - ymin)/(ymaximum - yminimum);	
	y_int = deltay*yscale;
	ny_int = (ymaximum - yminimum)/deltay + 1;

	/* plot grid */
	xg_drawline(hsvt_xgid,xmin,ymin,xmin,ymax,BLACK_ALL);
	xg_drawline(hsvt_xgid,xmax,ymin,xmax,ymax,BLACK_ALL);
	for (i=0;i<nx_int;i++)
		{
		xx = xmin + i*x_int;
		vx = xminimum + i*deltax;
		xg_drawline(hsvt_xgid,xx,ymin,xx,ymax,OVERLAY1_DASH);
		sprintf(string,"%1d",vx);
		xg_justify(hsvt_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(hsvt_xgid,xx-swidth/2,
			ymax+sascent+5,string,BLACK_ALL);
		}
	xg_drawline(hsvt_xgid,xmin,ymin,xmax,ymin,BLACK_ALL);
	xg_drawline(hsvt_xgid,xmin,ymax,xmax,ymax,BLACK_ALL);
	for (i=0;i<ny_int;i++)
		{
		yy = ymin + i*y_int;
		vy = yminimum + i*deltay;
		xg_drawline(hsvt_xgid,xmin,yy,xmax,yy,OVERLAY1_DASH);
		sprintf(string,"%1d",vy);
		xg_justify(hsvt_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(hsvt_xgid,xmin-swidth-5,
			yy+sascent/2,string,BLACK_ALL);
		}
	strcpy(string,"Water Velocity Profiles");
	xg_justify(hsvt_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(hsvt_xgid,xcen-swidth/2,
		ymin-2*sascent+10,string,BLACK_ALL);
	strcpy(string,"Water Velocity (m/s)");
	xg_justify(hsvt_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(hsvt_xgid,xcen-swidth/2,
		ymax+2*sascent+10,string,BLACK_ALL);
	strcpy(string,"Depth");
	xg_justify(hsvt_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(hsvt_xgid,xmin-2*swidth-10,
		ycen-sascent,string,BLACK_ALL);
	strcpy(string,"(m)");
	xg_drawstring(hsvt_xgid,xmin-2*swidth,
		ycen+sascent,string,BLACK_ALL);

	/* turn clipping on */
	xg_setclip(hsvt_xgid,xmin,ymin,(xmax-xmin),(ymax-ymin));

	/* plot display profiles */
	for (i=0;i<ndisplay;i++)
	  {
	  color = i%3 + 2;
	  for (j=0;j<profile_display[i].n;j++)
		{
		xx = xmin + (profile_display[i].velocity[j] - xminimum)*xscale;
		yy = ymin + (profile_display[i].depth[j] - yminimum)*yscale;
/*		xg_drawrectangle(hsvt_xgid, xx-2, yy-2, 4, 4, color);*/
		if (j > 0)
			xg_drawline(hsvt_xgid,xxo,yyo,xx,yy,color);
		xxo = xx;
		yyo = yy;
		}
	  }

	/* plot edit profile */
	if (edit == MB_YES)
	  for (j=0;j<profile_edit.n;j++)
		{
		xx = xmin + (profile_edit.velocity[j] - xminimum)*xscale;
		yy = ymin + (profile_edit.depth[j] - yminimum)*yscale;
		xg_fillrectangle(hsvt_xgid, xx-2, yy-2, 4, 4, OVERLAY2_DRAW);
		if (j > 0)
			xg_drawline(hsvt_xgid,xxo,yyo,xx,yy,OVERLAY2_DRAW);
		xxo = xx;
		yyo = yy;
		edit_x[j] = xx;
		edit_y[j] = yy;
		}

	/* now plot grid for Hydrosweep Residuals */

	/* turn clip mask back to whole canvas */
	xg_setclip(hsvt_xgid,borders[0],borders[2],
		borders[1]-borders[0],borders[3]-borders[2]);

	/* set scaling */
	xrmin = 2*margin;
	xrmax = borders[1] - margin;
	yrmin = borders[3] - 4*margin;
	yrmax = borders[3] - margin;
	xrcen = xrmin + (xrmax - xrmin)/2;
	yrcen = yrmin + (yrmax - yrmin)/2;
	xrminimum = -1.0;
	xrmaximum = 59.0;
	deltaxr = 0.1*(xrmaximum - xrminimum);
	xrscale = (xrmax - xrmin)/(xrmaximum - xrminimum);
	xr_int = deltaxr*xrscale;
	nxr_int = (xrmaximum - xrminimum)/deltaxr + 1;
	yrminimum = -resrange;
	yrmaximum = resrange;
	deltayr = 0.1*(yrmaximum - yrminimum);
	yrscale = (yrmax - yrmin)/(yrmaximum - yrminimum);	
	yr_int = deltayr*yrscale;
	nyr_int = (yrmaximum - yrminimum)/deltayr + 1;

	/* plot grid */
	xg_drawline(hsvt_xgid,xrmin,yrmin,xrmin,yrmax,BLACK_ALL);
	xg_drawline(hsvt_xgid,xrmax,yrmin,xrmax,yrmax,BLACK_ALL);
	for (i=0;i<nxr_int;i++)
		{
		xx = xrmin + i*xr_int;
		vx = xrminimum + i*deltaxr;
		xg_drawline(hsvt_xgid,xx,yrmin,xx,yrmax,OVERLAY1_DASH);
		sprintf(string,"%1d",vx);
		xg_justify(hsvt_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(hsvt_xgid,xx-swidth/2,
			yrmax+sascent+5,string,BLACK_ALL);
		}
	xg_drawline(hsvt_xgid,xrmin,yrmin,xrmax,yrmin,BLACK_ALL);
	xg_drawline(hsvt_xgid,xrmin,yrmax,xrmax,yrmax,BLACK_ALL);
	for (i=0;i<nyr_int;i++)
		{
		yy = yrmin + i*yr_int;
		vy = yrminimum + i*deltayr;
		xg_drawline(hsvt_xgid,xrmin,yy,xrmax,yy,OVERLAY1_DASH);
		sprintf(string,"%1d",vy);
		xg_justify(hsvt_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(hsvt_xgid,xrmin-swidth-5,
			yy+sascent/2,string,BLACK_ALL);
		}
	if (nbuffer > 0)
		{
		sprintf(string,"Depth Range:  minimum: %f m   maximum: %f m",
			bath_min, bath_max);
		xg_justify(hsvt_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(hsvt_xgid,xrcen-swidth/2,
			yrmin-4*sascent+10,string,BLACK_ALL);
		}
	strcpy(string,"Hydrosweep Bathymetry Beam Residuals");
	xg_justify(hsvt_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(hsvt_xgid,xrcen-swidth/2,
		yrmin-2*sascent+10,string,BLACK_ALL);
	strcpy(string,"Hydrosweep Beam Number");
	xg_justify(hsvt_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(hsvt_xgid,xrcen-swidth/2,
		yrmax+2*sascent+10,string,BLACK_ALL);
	strcpy(string,"Residual");
	xg_justify(hsvt_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(hsvt_xgid,xrmin-swidth-20,
		yrcen-sascent,string,BLACK_ALL);
	strcpy(string,"(m)");
	xg_drawstring(hsvt_xgid,xrmin-swidth,
		yrcen+sascent,string,BLACK_ALL);

	/* turn clipping on for residual plot box */
	xg_setclip(hsvt_xgid,xrmin,yrmin,(xrmax-xrmin),(yrmax-yrmin));

	/* plot residuals */
	if (nbuffer > 0)
	  for (i=0;i<MBSYS_HSDS_BEAMS;i++)
	    {
	    if (nresidual[i] > 0)
		{
		xx = xrmin + (i - xrminimum)*xrscale;
		yy = yrmin + (residual[i] - yrminimum)*yrscale;
		xg_fillrectangle(hsvt_xgid, xx-2, yy-2, 4, 4, OVERLAY2_DRAW);
		if (i > 0 && nresidual[i-1] > 0)
			xg_drawline(hsvt_xgid,xxo,yyo,xx,yy,OVERLAY2_DRAW);
		xxo = xx;
		yyo = yy;
		}
	     }

	/* turn clipping on for velocity profile box */
	xg_setclip(hsvt_xgid,xmin,ymin,(xmax-xmin),(ymax-ymin));

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_action_mouse_down(x,y)
int	x;
int	y;
{
	/* local variables */
	char	*function_name = "hsvt_action_mouse_down";
	int	status = MB_SUCCESS;
	double	distance, distance_min;
	int	found;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input values:\n");
		fprintf(stderr,"dbg2       x:            %d\n",x);
		fprintf(stderr,"dbg2       y:            %d\n",y);
		}

	/* select node if possible */
	if (x >= xmin && x <= xmax && y >= ymin && y <= ymax)
		{

		/* find the closest node */
		distance_min = 20000.0;
		active = -1;
		for (i=0;i<profile_edit.n;i++)
			{
			distance = (edit_x[i] - x)*(edit_x[i] - x) 
				+ (edit_y[i] - y)*(edit_y[i] - y);
			if (distance < distance_min)
				{
				distance_min = distance;
				active = i;
				}
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_action_mouse_up(x,y)
int	x;
int	y;
{
	/* local variables */
	char	*function_name = "hsvt_action_mouse_up";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input values:\n");
		fprintf(stderr,"dbg2       x:            %d\n",x);
		fprintf(stderr,"dbg2       y:            %d\n",y);
		}

	/* relocate and deselect node if selected */
	if (active > 0)
		{
		active = -1;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_action_mouse_drag(x,y)
int	x;
int	y;
{
	/* local variables */
	char	*function_name = "hsvt_action_mouse_drag";
	int	status = MB_SUCCESS;
	int	ylim_min, ylim_max;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input values:\n");
		fprintf(stderr,"dbg2       x:            %d\n",x);
		fprintf(stderr,"dbg2       y:            %d\n",y);
		}

	/* relocate node if selected */
	if (active > -1 && 
		x >= xmin && x <= xmax && y >= ymin && y <= ymax)
		{
		/* find upper and lower bounds for current node */
		if (active == 0)
			ylim_min = ymin;
		else
			ylim_min = edit_y[active - 1];
		if (active == profile_edit.n - 1)
			ylim_max = ymax;
		else
			ylim_max = edit_y[active + 1];

		/* get new location */
		if (x <= xmin)
			x = xmin + 1;
		if (x >= xmax)
			x = xmax - 1;
		if (y <= ylim_min)
			y = ylim_min + 1;
		if (y >= ylim_max)
			y = ylim_max;
		if (active == 0)
			y = ymin;

		/* unplot the current ping */
		xg_fillrectangle(hsvt_xgid, edit_x[active]-2, 
			edit_y[active]-2, 4, 4, OVERLAY2_CLEAR);
		if (active > 0)
			xg_drawline(hsvt_xgid,
				edit_x[active-1],edit_y[active-1],
				edit_x[active],edit_y[active],
				OVERLAY2_CLEAR);
		if (active < profile_edit.n - 1)
			xg_drawline(hsvt_xgid,
				edit_x[active],edit_y[active],
				edit_x[active+1],edit_y[active+1],
				OVERLAY2_CLEAR);

		/* get new location values */
		edit_x[active] = x;
		edit_y[active] = y;
		profile_edit.velocity[active] = (x - xmin)/xscale + xminimum;
		profile_edit.depth[active] = (y - ymin)/yscale + yminimum;

		/* replot the current ping */
		if (active > 0)
			xg_drawline(hsvt_xgid,
				edit_x[active-1],edit_y[active-1],
				edit_x[active],edit_y[active],
				OVERLAY2_DRAW);
		if (active < profile_edit.n - 1)
			xg_drawline(hsvt_xgid,
				edit_x[active],edit_y[active],
				edit_x[active+1],edit_y[active+1],
				OVERLAY2_DRAW);
		if (active > 0)
			xg_fillrectangle(hsvt_xgid, edit_x[active-1]-2, 
				edit_y[active-1]-2, 4, 4, OVERLAY2_DRAW);
		xg_fillrectangle(hsvt_xgid, edit_x[active]-2, 
			edit_y[active]-2, 4, 4, OVERLAY2_DRAW);
		if (active < profile_edit.n - 1)
			xg_fillrectangle(hsvt_xgid, edit_x[active+1]-2, 
				edit_y[active+1]-2, 4, 4, OVERLAY2_DRAW);
		}
	else
		status = MB_FAILURE;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_open_hydrosweep_file(file,form)
char	*file;
int	form;
{
	/* local variables */
	char	*function_name = "hsvt_open_hydrosweep_file";
	int	status = MB_SUCCESS;
	char	*suffix;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file:        %s\n",file);
		fprintf(stderr,"dbg2       format:      %d\n",form);
		}

	/* check for proper format */
	if (format != MBF_HSATLRAW && format != MBF_HSLDEOIH)
		{
		fprintf(stderr,"\nProgram <%s> requires complete Hydrosweep DS data stream\n",program_name);
		fprintf(stderr,"!!Format %d is unacceptable, only formats %d and %d can be used\n",format,MBF_HSATLRAW,MBF_HSLDEOIH);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		status = MB_FAILURE;
		return(status);
		}

	/* deallocate previously loaded data, if any */
	if (nbuffer > 0)
		{
		mb_free(verbose,bath,&error);
		mb_free(verbose,bathdist,&error);
		mb_free(verbose,back,&error);
		mb_free(verbose,backdist,&error);
		mb_buffer_close(verbose,buff_ptr,&error);
		}

	/* initialize reading the input hydrosweep file */
	format = form;
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_back,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		status = MB_FAILURE;
		return(status);
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(int),&bath,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(int),&bathdist,&error);
	status = mb_malloc(verbose,beams_back*sizeof(int),&back,&error);
	status = mb_malloc(verbose,beams_back*sizeof(int),&backdist,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize the buffer */
	status = mb_buffer_init(verbose,&buff_ptr,&error);
	nbuffer = 0;

	/* load data into buffer */
	status = mb_buffer_load(verbose,buff_ptr,mbio_ptr,buffer_size,
			&nload,&nbuffer,&error);

	/* close the file */
	status = mb_close(verbose,mbio_ptr,&error);

	/* set up for raytracing */
	if (status == MB_SUCCESS)
		status = hsvt_setup_raytracing();

	/* output info */
	if (verbose >= 0)
		{
		if (status == MB_SUCCESS)
			fprintf(stderr,"\nHydrosweep File <%s> read\n",file);
		else
			fprintf(stderr,"\nHydrosweep File <%s> not read\n",file);
		fprintf(stderr,"Multibeam Data Format ID:   %d\n",format);
		fprintf(stderr,"Records loaded into buffer: %d\n",nload);
		fprintf(stderr,"Records in buffer:          %d\n",nbuffer);
		}

	/* process the data */
	if (status == MB_SUCCESS)
		status = hsvt_process_hydrosweep();

	/* plot everything */
	hsvt_plot();

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_setup_raytracing()
{
	/* local variables */
	char	*function_name = "hsvt_setup_raytracing";
	int	status = MB_SUCCESS;
	double	*ttime;
	double	*dist;
	double	*vel;
	double	*dep;
	int	nvel;
	int	center_beam;
	double	dr, dx;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* check for velocity profile */
	if (profile_edit.n <= 0)
		{
		fprintf(stderr,"\nNo edit velocity profile available - Raytracing initialization aborted.\n");
		status = MB_FAILURE;
		return(status);
		}

	/* construct layered velocity model from discrete model */
	nvel = profile_edit.n;
	vel = profile_edit.velocity_layer;
	dep = profile_edit.depth;
	for (i=0;i<nvel-1;i++)
		vel[i] = 0.5*(profile_edit.velocity[i] 
				+ profile_edit.velocity[i+1]);
	vel[nvel-1] = 0.0;

	/* set the angle increment between survey ping beams */
	center_beam = MBSYS_HSDS_BEAMS/2;
	for (i=0;i<MBSYS_HSDS_BEAMS;i++)
		{
		s_angle[i] = (i-center_beam)*dangle;
		s_p[i] = sin(DTR*s_angle[i])/vel[0];
		}

	/* set up the raytracing tables for survey pings */
	for (i=0;i<MBSYS_HSDS_BEAMS;i++)
		{
		ttime = s_ttime_tab[i];
		dist = s_dist_tab[i];
		ttime[0] = 0.0;
		dist[0] = 0.0;
		for (j=0;j<nvel-1;j++)
			{
			dr = (dep[j+1] - dep[j])/sqrt(1. 
				- s_p[i]*s_p[i]*vel[j]*vel[j]);
			dx = dr*s_p[i]*vel[j];
			ttime[j+1] = ttime[j] + 2.*dr/vel[j];
			dist[j+1] = dist[j] + dx;
			}

		/* output some debug values */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Raytracing table created for survey beam %d in program <%s>:\n",i,program_name);
			fprintf(stderr,"dbg5       angle: %f\n",s_angle[i]);
			fprintf(stderr,"dbg5       p:     %f\n",s_p[i]);
			fprintf(stderr,"dbg5      beam    depth      vel        time      dist\n",j,dep[j],vel[j],ttime[j],dist[j]);
			for (j=0;j<nvel;j++)
				fprintf(stderr,"dbg5       %2d   %8.2f   %7.2f   %8.2f  %9.2f\n",j,dep[j],vel[j],ttime[j],dist[j]);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int hsvt_process_hydrosweep()
{
	/* local variables */
	char	*function_name = "hsvt_process_hydrosweep";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mbsys_hsds_struct *store;
	double	*ttime;
	double	*dist;
	double	*vel;
	double	*dep;
	int	nvel;
	int	flagged;
	double	tt, factor, zz, xx;
	double	sx, sy, sxx, sxy;
	double	delta, a, b;
	int	ns;
	double	depth_predict, res;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* check for data and velocity profile */
	if (profile_edit.n <= 0)
		{
		fprintf(stderr,"\nNo edit velocity profile available - Hydrosweep processing aborted.\n");
		status = MB_FAILURE;
		return(status);
		}
	if (nbuffer <= 0)
		{
		fprintf(stderr,"\nNo Hydrosweep data available - Hydrosweep processing aborted.\n");
		status = MB_FAILURE;
		return(status);
		}

	/* initialize residuals */
	for (i=0;i<MBSYS_HSDS_BEAMS;i++)
		{
		residual[i] = 0.0;
		nresidual[i] = 0;
		}

	/* initialize min-max variables */
	bath_min = 10000.0;
	bath_max = 0.0;

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;
	nvel = profile_edit.n;
	vel = profile_edit.velocity_layer;
	dep = profile_edit.depth;

	/* loop over the data records */
	for (k=0;k<nbuffer;k++)
		{
		/* get store pointer and kind */
		store = (struct mbsys_hsds_struct *) buff->buffer[k];

		/* initialize linear fit variables */
		sx = 0.0;
		sy = 0.0;
		sxx = 0.0;
		sxy = 0.0;
		ns = 0;
		depth_predict = 0.0;
		res = 0.0;

		/* output some debug values */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Data record used in program <%s>:\n",program_name);
			fprintf(stderr,"dbg5       record %d  kind: %d",
				k,buff->buffer_kind[k]);
			if (buff->buffer_kind[k] == MB_DATA_COMMENT)
				fprintf(stderr,"  comment: %s\n",store->comment);
			else
				fprintf(stderr,"  time: %4d %2d %2d %2d:%2d:%2d\n",
					store->year, store->month, store->day,
					store->hour, store->minute, store->second);
			}

		/* loop over the beams */
		if (buff->buffer_kind[k] == MB_DATA_DATA)
		  for (i=0;i<MBSYS_HSDS_BEAMS;i++)
		    {
		    ttime = s_ttime_tab[i];
		    dist = s_dist_tab[i];

		    /* output some debug values */
		    if (verbose >= 5)
			fprintf(stderr,"dbg5       %2d  %5d %5d",
				i,store->distance[i],store->depth[i]);

		    /* calculate the depths 
			and crosstrack distances */ 
		    if (store->depth[i] < 0)
			flagged = MB_YES;
		    else
			flagged = MB_NO;
		    store->depth[i] = 0;
		    store->distance[i] = 0;
		    if (store->time[i] > 0)
			{
			tt = store->time_scale*store->time[i];
			for (j=0;j<nvel-1;j++)
			  if (tt > ttime[j] && tt <= ttime[j+1])
				{
				factor = (tt - ttime[j])
					/(ttime[j+1] - ttime[j]);
				zz = dep[j] 
					+ factor*(dep[j+1] - dep[j]) + 5.5;
				xx = dist[j]
					+ factor*(dist[j+1] - dist[j]);
				store->distance[i] = (int) (xx + 0.5);
				store->depth[i] = (int) (zz + 0.5);
				if (flagged == MB_YES)
					store->depth[i] = -store->depth[i];
				else
					{
					if (zz < bath_min)
						bath_min = zz;
					if (zz > bath_max)
						bath_max = zz;
					}
				}
			}

		    /* output some debug values */
		    if (verbose >= 5)
			fprintf(stderr,"  %5d %5d\n",
				store->distance[i],store->depth[i]);

		    /* get sums for linear fit */
		    if (store->depth[i] > 0)
			{
			sx += store->distance[i];
			sy += store->depth[i];
			sxx += store->distance[i]*store->distance[i];
			sxy += store->distance[i]*store->depth[i];
			ns++;
			}
		    }

		/* get linear fit to ping */
		if (ns > 0)
			{
			delta = ns*sxx - sx*sx;
			a = (sxx*sy - sx*sxy)/delta;
			b = (ns*sxy - sx*sy)/delta;
			}

		/* get residuals */
		if (ns > 0)
		  for (i=0;i<MBSYS_HSDS_BEAMS;i++)
		    if (store->depth[i] > 0)
			{
			depth_predict = a + b*store->distance[i];
			res = store->depth[i] - depth_predict;
			residual[i] += res;
			nresidual[i]++;

			/* output some debug values */
			if (verbose >= 5)
		    		fprintf(stderr,"dbg5       %d %d %d %f %f\n",
					i,store->distance[i],store->depth[i],
					depth_predict,res);
			}
		}

	/* calculate final residuals */
	for (i=0;i<MBSYS_HSDS_BEAMS;i++)
		if (nresidual[i] > 0)
			residual[i] = residual[i]/nresidual[i];

	/* output residuals and stuff */
	if (verbose >= 1)
		{
		fprintf(stderr,"\nCurrent Hydrosweep Depth Range:\n");
		fprintf(stderr,"\tminimum depth: %f\n",bath_min);
		fprintf(stderr,"\tmaximum depth: %f\n",bath_max);
		fprintf(stderr,"\nHydrosweep Bathymetry Beam Residuals:\n");
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			fprintf(stderr,"beam: %2d   residual: %f  calculations: %d\n",
				i,residual[i],nresidual[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
