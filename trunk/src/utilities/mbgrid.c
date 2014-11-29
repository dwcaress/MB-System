/*--------------------------------------------------------------------
 *    The MB-system:	mbgrid.c	5/2/94
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
 * MBGRID is an utility used to grid bathymetry, amplitude, or
 * sidescan data contained in a set of swath sonar data files.
 * This program uses one of four algorithms (gaussian weighted mean,
 * median filter, minimum filter, maximum filter) to grid regions
 * covered by swaths and then fills in gaps between
 * the swaths (to the degree specified by the user) using a minimum
 * curvature algorithm.
 *
 * The April 1995 version reinstated the use of the IGPP/SIO zgrid routine
 * for thin plate spline interpolation. The zgrid code has been
 * translated from Fortran to C. The zgrid algorithm is much
 * faster than the Wessel and Smith minimum curvature algorithm
 * from the GMT program surface used in recent versions of mbgrid.
 *
 * The July 2002 version allows the creation of grids using
 * UTM eastings and northings rather than uniformly spaced
 * in longitude and latitude.
 *
 * Author:	D. W. Caress
 * Date:	February 22, 1993
 * Rewrite:	May 2, 1994
 * Rerewrite:	April 25, 1995
 * Rererewrite:	January 2, 1996
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* GMT include files */
#include "gmt.h"

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_info.h"
#include "mb_aux.h"

/* gridding algorithms */
#define	MBGRID_WEIGHTED_MEAN                1
#define	MBGRID_MEDIAN_FILTER                2
#define	MBGRID_MINIMUM_FILTER               3
#define	MBGRID_MAXIMUM_FILTER               4
#define	MBGRID_WEIGHTED_FOOTPRINT_SLOPE     5
#define	MBGRID_WEIGHTED_FOOTPRINT           6

/* grid format definitions */
#define	MBGRID_ASCII	1
#define	MBGRID_OLDGRD	2
#define	MBGRID_CDFGRD	3
#define	MBGRID_ARCASCII	4
#define	MBGRID_GMTGRD	100

/* gridded data type */
#define	MBGRID_DATA_BATHYMETRY	1
#define	MBGRID_DATA_TOPOGRAPHY	2
#define	MBGRID_DATA_AMPLITUDE	3
#define	MBGRID_DATA_SIDESCAN	4

/* flag for no data in grid */
#define	NO_DATA_FLAG	99999

/* number of data to be allocated at a time */
#define	REALLOC_STEP_SIZE	25

/* usage of footprint based weight */
#define MBGRID_USE_NO		0
#define MBGRID_USE_YES		1
#define MBGRID_USE_CONDITIONAL	2

/* interpolation mode */
#define MBGRID_INTERP_NONE	0
#define MBGRID_INTERP_GAP	1
#define MBGRID_INTERP_NEAR	2
#define MBGRID_INTERP_ALL	3

/* comparison threshold */
#define MBGRID_TINY		0.00000001

/* interpolation algorithm
	The code is set to use either of two
	algorithms for 2D thin plate spline
	interpolation. If the USESURFACE preprocessor
	define is defined then
	the code will use the surface algorithm
	from GMT. If not, then the zgrid
	algorithm will be used.
	- The default is to use zgrid - to
	change this uncomment the define below. */
/* #define USESURFACE */

/* approximate complementary error function */
double erfcc();
double mbgrid_erf();

int write_ascii(int verbose, char *outfile, float *grid,
		int nx, int ny,
		double xmin, double xmax, double ymin, double ymax,
		double dx, double dy, int *error);
int write_arcascii(int verbose, char *outfile, float *grid,
		int nx, int ny,
		double xmin, double xmax, double ymin, double ymax,
		double dx, double dy, double nodata, int *error);
int write_oldgrd(int verbose, char *outfile, float *grid,
		int nx, int ny,
		double xmin, double xmax, double ymin, double ymax,
		double dx, double dy, int *error);
int write_cdfgrd(int verbose, char *outfile, float *grid,
		int nx, int ny,
		double xmin, double xmax, double ymin, double ymax,
		double zmin, double zmax, double dx, double dy,
		char *xlab, char *ylab, char *zlab, char *titl,
		char *projection, int argc, char **argv,
		int *error);
int mbgrid_weight(int verbose, double foot_a, double foot_b,
		    double pcx, double pcy, double dx, double dy,
		    double *px, double *py,
		    double *weight, int *use, int *error);

/* output stream for basic stuff (stdout if verbose <= 1,
	stderr if verbose > 1) */
FILE	*outfp;

/* program identifiers */
static char rcs_id[] = "$Id$";
char program_name[] = "mbgrid";
char help_message[] =  "mbgrid is an utility used to grid bathymetry, amplitude, or \nsidescan data contained in a set of swath sonar data files.  \nThis program uses one of four algorithms (gaussian weighted mean, \nmedian filter, minimum filter, maximum filter) to grid regions \ncovered swaths and then fills in gaps between \nthe swaths (to the degree specified by the user) using a minimum\ncurvature algorithm.";
char usage_message[] = "mbgrid -Ifilelist -Oroot \
[-Rwest/east/south/north -Rfactor -Adatatype\n\
          -Bborder -Cclip[/mode[/tension]] -Dxdim/ydim -Edx/dy/units[!] -F\n\
          -Ggridkind -H -Jprojection -Llonflip -M -N -Ppings -Sspeed\n\
          -Utime -V -Wscale -Xextend]";
/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message = NULL;

	/* MBIO read control parameters */
	int	format;
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
	char	file[MB_PATH_MAXLINE];
	int	file_in_bounds;
	void	*mbio_ptr = NULL;
	struct mb_io_struct *mb_io_ptr = NULL;
        int     topo_type;

	/* mbgrid control variables */
	char	filelist[MB_PATH_MAXLINE];
	char	fileroot[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	xdim = 0;
	int	ydim = 0;
	int	spacing_priority = MB_NO;
	int	set_dimensions = MB_NO;
	int	set_spacing = MB_NO;
	double	dx_set = 0.0;
	double	dy_set = 0.0;
	double	dx = 0.0;
	double	dy = 0.0;
	char	units[MB_PATH_MAXLINE];
	int	clip = 0;
	int	clipmode = MBGRID_INTERP_NONE;
#ifdef USESURFACE
	double	tension = 0.35;
#else
	double	tension = 0.0;
#endif
	int	grid_mode = MBGRID_WEIGHTED_MEAN;
	int	datatype = MBGRID_DATA_BATHYMETRY;
	char	gridkindstring[MB_PATH_MAXLINE];
	int	gridkind = MBGRID_GMTGRD;
	int	more = MB_NO;
	int	use_NaN = MB_NO;
	double	clipvalue = NO_DATA_FLAG;
	float	outclipvalue = NO_DATA_FLAG;
	double	scale = 1.0;
        double  boundsfactor = 0.0;
        int     setborder = MB_NO;
	double	border = 0.0;
	double	extend = 0.0;
	int	check_time = MB_NO;
	int	first_in_stays = MB_YES;
	double	timediff = 300.0;
	int	rformat;
	int	pstatus;
	char	path[MB_PATH_MAXLINE];
	char	ppath[MB_PATH_MAXLINE];
	char	rfile[MB_PATH_MAXLINE];
	char	ofile[MB_PATH_MAXLINE];
	char	dfile[MB_PATH_MAXLINE];
	char	plot_cmd[MB_COMMENT_MAXLINE];
	char	plot_stdout[MB_COMMENT_MAXLINE];
	int	plot_status;

	int	grdrasterid = 0;
	char	backgroundfile[MB_PATH_MAXLINE];
	char	backgroundfileuse[MB_PATH_MAXLINE];

	/* mbio read values */
	int	rpings;
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
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathlon = NULL;
	double	*bathlat = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*sslon = NULL;
	double	*sslat = NULL;
	char	comment[MB_COMMENT_MAXLINE];
	struct mb_info_struct mb_info;
	int	formatread;

	/* lon,lat,value triples variables */
	double	tlon;
	double	tlat;
	double	tvalue;

	/* grid variables */
	double	gbnd[4], wbnd[4], obnd[4];
	int	gbndset = MB_NO;
	double	xlon, ylat, xx, yy;
	double	factor, weight, topofactor;
	int	gxdim, gydim, offx, offy, xtradim;
	double	sbnd[4], sdx, sdy;
	int	sclip;
	int	sxdim, sydim;
	double	*grid = NULL;
	double	*norm = NULL;
	double	*sigma = NULL;
	double	*firsttime = NULL;
	double	*gridsmall = NULL;
#ifdef USESURFACE
	float	*bxdata = NULL;
	float	*bydata = NULL;
	float	*bzdata = NULL;
	float	*sxdata = NULL;
	float	*sydata = NULL;
	float	*szdata = NULL;
#else
	float	*bdata = NULL;
	float	*sdata = NULL;
	float	*work1 = NULL;
	int	*work2 = NULL;
	int	*work3 = NULL;
#endif
        double  bdata_origin_x, bdata_origin_y;
	float	*output = NULL;
	float	*sgrid = NULL;
	int	*num = NULL;
	int	*cnt = NULL;
	float	xmin, ymin, ddx, ddy, zflag, cay;
	double	**data;
	double	*value = NULL;
	int	ndata, ndatafile, nbackground, nbackground_alloc;
	int	time_ok;
	double	zmin, zmax, zclip;
	int	nmax;
	double	smin, smax;
	int	nbinset, nbinzero, nbinspline, nbinbackground;
	int	bathy_in_feet = MB_NO;

	/* projected grid parameters */
	int	use_projection = MB_NO;
	int	projection_pars_f = MB_NO;
	double	reference_lon, reference_lat;
	int	utm_zone = 1;
	char	projection_pars[MB_PATH_MAXLINE];
	char	projection_id[MB_PATH_MAXLINE];
	int	proj_status;
	void	*pjptr;
	double	deglontokm, deglattokm;
	double	mtodeglon, mtodeglat;

	/* output char strings */
	char	xlabel[MB_PATH_MAXLINE];
	char	ylabel[MB_PATH_MAXLINE];
	char	zlabel[MB_PATH_MAXLINE];
	char	title[MB_PATH_MAXLINE];
	char	nlabel[MB_PATH_MAXLINE];
	char	sdlabel[MB_PATH_MAXLINE];

	/* variables needed to handle Not-a-Number values */
	float	NaN;

	/* other variables */
	FILE	*dfp, *rfp;
	int	i, j, k, ii, jj, iii, jjj, kkk, ir, n;
	int	i1, i2, j1, j2, k1, k2;
	double	r;
	int	dmask[9];
	int	kgrid, kout, kint, ib, ix, iy;
	int	ix1, ix2, iy1, iy2, isx, isy;
	int	pid;

	double	foot_dx, foot_dy, foot_dxn, foot_dyn;
	double	foot_lateral, foot_range, foot_theta;
	double	foot_dtheta, foot_dphi;
	double	foot_hwidth, foot_hlength;
	int	foot_wix, foot_wiy, foot_lix, foot_liy, foot_dix, foot_diy;
	double	dzdx, dzdy, sbath;
	double	xx0, yy0, bdx, bdy, xx1, xx2, yy1, yy2;
	double	prx[5], pry[5];
	int	use_weight;
	int	fork_status;
        char    *bufptr;
        size_t  freadsize;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input and output */
	strcpy (filelist, "datalist.mb-1");

	/* initialize some values */
	gridkindstring[0] = '\0';
	strcpy(fileroot,"grid");
	strcpy(projection_id,"Geographic");
	gbnd[0] = 0.0;
	gbnd[1] = 0.0;
	gbnd[2] = 0.0;
	gbnd[3] = 0.0;
	xdim = 101;
	ydim = 101;
	gxdim = 0;
	gydim = 0;
 	pid = getpid();

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:F:f:G:g:HhI:i:J:j:K:k:L:l:MmNnO:o:P:p:QqR:r:S:s:T:t:U:u:VvW:w:X:x:")) != -1)
	  switch (c)
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%d", &datatype);
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%lf", &border);
                        setborder = MB_YES;
			flag++;
			break;
		case 'C':
		case 'c':
			n = sscanf (optarg,"%d/%d/%lf", &clip, &clipmode,&tension);
			if (n < 1)
				clipmode = MBGRID_INTERP_NONE;
			else if (n == 1 && clip > 0)
				clipmode = MBGRID_INTERP_GAP;
			else if (n == 1)
				clipmode = MBGRID_INTERP_NONE;
			else if (clip > 0 && clipmode < 0)
				clipmode = MBGRID_INTERP_GAP;
			else if (clipmode >= 3)
				clipmode = MBGRID_INTERP_ALL;
                        if (n < 3)
                                {
#ifdef USESURFACE
                                tension = 0.35;
#else
                                tension = 0.0;
#endif
                                }
			flag++;
			break;
		case 'D':
		case 'd':
			n = sscanf (optarg,"%d/%d", &xdim, &ydim);
			if (n == 2)
				set_dimensions = MB_YES;
			flag++;
			break;
		case 'E':
		case 'e':
			if (optarg[strlen(optarg)-1] == '!')
			    {
			    spacing_priority = MB_YES;
			    optarg[strlen(optarg)-1] = '\0';
			    }
			n = sscanf (optarg,"%lf/%lf/%s", &dx_set, &dy_set, units);
			if (n > 1)
				set_spacing = MB_YES;
			if (n < 3)
				strcpy(units, "meters");
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &grid_mode);
			flag++;
			break;
		case 'G':
		case 'g':
			if (optarg[0] == '=')
				{
				gridkind = MBGRID_GMTGRD;
				strcpy(gridkindstring, optarg);
				}
			else
				{
				sscanf (optarg,"%d", &gridkind);
				if (gridkind == MBGRID_CDFGRD)
					{
					gridkind = MBGRID_GMTGRD;
					gridkindstring[0] = '\0';
					}
				else if (gridkind > MBGRID_GMTGRD)
					{
					sprintf(gridkindstring, "=%d", (gridkind - 100));
					gridkind = MBGRID_GMTGRD;
					}
				}
			flag++;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", filelist);
			flag++;
			break;
		case 'J':
		case 'j':
			sscanf (optarg,"%s", projection_pars);
			projection_pars_f = MB_YES;
			flag++;
			break;
		case 'K':
		case 'k':
			sscanf (optarg,"%s", backgroundfile);
			if ((grdrasterid = atoi(backgroundfile)) <= 0)
				grdrasterid = -1;
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'M':
		case 'm':
			more = MB_YES;
			flag++;
			break;
		case 'N':
		case 'n':
			use_NaN = MB_YES;
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", fileroot);
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%d", &pings);
			flag++;
			break;
		case 'Q':
		case 'q':
			bathy_in_feet = MB_YES;
			flag++;
			break;
		case 'R':
		case 'r':
                        if (strchr(optarg,'/') == NULL)
                            {
                            sscanf (optarg,"%lf", &boundsfactor);
                            if (boundsfactor <= 1.0)
                                boundsfactor = 0.0;
                            }
			else
                            {
                            mb_get_bounds(optarg, gbnd);
                            gbndset = MB_YES;
                            }
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%lf", &speedmin);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &tension);
			flag++;
			break;
		case 'U':
		case 'u':
			sscanf (optarg,"%lf", &timediff);
			timediff = 60*timediff;
			check_time = MB_YES;
			if (timediff < 0.0)
				{
				timediff = fabs(timediff);
				first_in_stays = MB_NO;
				}
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'W':
		case 'w':
			sscanf (optarg,"%lf", &scale);
			flag++;
			break;
		case 'X':
		case 'x':
			sscanf (optarg,"%lf", &extend);
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* set output stream to stdout or stderr */
	if (verbose >= 2)
	    outfp = stderr;
	else
	    outfp = stdout;

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(outfp,"usage: %s\n", usage_message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(outfp,"\nProgram %s\n",program_name);
		fprintf(outfp,"Version %s\n",rcs_id);
		fprintf(outfp,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Program <%s>\n",program_name);
		fprintf(outfp,"dbg2  Version %s\n",rcs_id);
		fprintf(outfp,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(outfp,"dbg2  Control Parameters:\n");
		fprintf(outfp,"dbg2       verbose:              %d\n",verbose);
		fprintf(outfp,"dbg2       help:                 %d\n",help);
		fprintf(outfp,"dbg2       pings:                %d\n",pings);
		fprintf(outfp,"dbg2       lonflip:              %d\n",lonflip);
		fprintf(outfp,"dbg2       btime_i[0]:           %d\n",btime_i[0]);
		fprintf(outfp,"dbg2       btime_i[1]:           %d\n",btime_i[1]);
		fprintf(outfp,"dbg2       btime_i[2]:           %d\n",btime_i[2]);
		fprintf(outfp,"dbg2       btime_i[3]:           %d\n",btime_i[3]);
		fprintf(outfp,"dbg2       btime_i[4]:           %d\n",btime_i[4]);
		fprintf(outfp,"dbg2       btime_i[5]:           %d\n",btime_i[5]);
		fprintf(outfp,"dbg2       btime_i[6]:           %d\n",btime_i[6]);
		fprintf(outfp,"dbg2       etime_i[0]:           %d\n",etime_i[0]);
		fprintf(outfp,"dbg2       etime_i[1]:           %d\n",etime_i[1]);
		fprintf(outfp,"dbg2       etime_i[2]:           %d\n",etime_i[2]);
		fprintf(outfp,"dbg2       etime_i[3]:           %d\n",etime_i[3]);
		fprintf(outfp,"dbg2       etime_i[4]:           %d\n",etime_i[4]);
		fprintf(outfp,"dbg2       etime_i[5]:           %d\n",etime_i[5]);
		fprintf(outfp,"dbg2       etime_i[6]:           %d\n",etime_i[6]);
		fprintf(outfp,"dbg2       speedmin:             %f\n",speedmin);
		fprintf(outfp,"dbg2       timegap:              %f\n",timegap);
		fprintf(outfp,"dbg2       file list:            %s\n",filelist);
		fprintf(outfp,"dbg2       output file root:     %s\n",fileroot);
		fprintf(outfp,"dbg2       grid x dimension:     %d\n",xdim);
		fprintf(outfp,"dbg2       grid y dimension:     %d\n",ydim);
		fprintf(outfp,"dbg2       grid x spacing:       %f\n",dx);
		fprintf(outfp,"dbg2       grid y spacing:       %f\n",dy);
		fprintf(outfp,"dbg2       grid bounds[0]:       %f\n",gbnd[0]);
		fprintf(outfp,"dbg2       grid bounds[1]:       %f\n",gbnd[1]);
		fprintf(outfp,"dbg2       grid bounds[2]:       %f\n",gbnd[2]);
		fprintf(outfp,"dbg2       grid bounds[3]:       %f\n",gbnd[3]);
		fprintf(outfp,"dbg2       boundsfactor:         %f\n",boundsfactor);
		fprintf(outfp,"dbg2       clipmode:             %d\n",clipmode);
		fprintf(outfp,"dbg2       clip:                 %d\n",clip);
		fprintf(outfp,"dbg2       tension:              %f\n",tension);
		fprintf(outfp,"dbg2       grdraster background: %d\n",grdrasterid);
		fprintf(outfp,"dbg2       backgroundfile:       %s\n",backgroundfile);
		fprintf(outfp,"dbg2       more:                 %d\n",more);
		fprintf(outfp,"dbg2       use_NaN:              %d\n",use_NaN);
		fprintf(outfp,"dbg2       grid_mode:            %d\n",grid_mode);
		fprintf(outfp,"dbg2       data type:            %d\n",datatype);
		fprintf(outfp,"dbg2       grid format:          %d\n",gridkind);
		if (gridkind == MBGRID_GMTGRD)
		fprintf(outfp,"dbg2       gmt grid format id:   %s\n",gridkindstring);
		fprintf(outfp,"dbg2       scale:                %f\n",scale);
		fprintf(outfp,"dbg2       timediff:             %f\n",timediff);
		fprintf(outfp,"dbg2       setborder:            %d\n",setborder);
		fprintf(outfp,"dbg2       border:               %f\n",border);
		fprintf(outfp,"dbg2       extend:               %f\n",extend);
		fprintf(outfp,"dbg2       bathy_in_feet:        %d\n",bathy_in_feet);
		fprintf(outfp,"dbg2       projection_pars:      %s\n",projection_pars);
		fprintf(outfp,"dbg2       proj flag 1:          %d\n",projection_pars_f);
		fprintf(outfp,"dbg2       projection_id:        %s\n",projection_id);
		fprintf(outfp,"dbg2       utm_zone:             %d\n",utm_zone);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(outfp,"\n%s\n",help_message);
		fprintf(outfp,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* if bounds not set get bounds of input data */
	if (gbndset == MB_NO)
		{
		formatread = -1;
		status = mb_get_info_datalist(verbose, filelist, &formatread,
				&mb_info, lonflip, &error);

		gbnd[0] = mb_info.lon_min;
		gbnd[1] = mb_info.lon_max;
		gbnd[2] = mb_info.lat_min;
		gbnd[3] = mb_info.lat_max;
		gbndset = MB_YES;

		if (set_spacing == MB_NO && set_dimensions == MB_NO)
			{
			dx_set = 0.02 * mb_info.altitude_max;
			dy_set = 0.02 * mb_info.altitude_max;
			set_spacing = MB_YES;
			strcpy(units, "meters");
			}
		}

        /* if requested expand the grid bounds */
        if (boundsfactor > 1.0)
                {
                xx1 = 0.5 * (boundsfactor - 1.0) * (gbnd[1] - gbnd[0]);
                yy1 = 0.5 * (boundsfactor - 1.0) * (gbnd[3] - gbnd[2]);
		gbnd[0] -= xx1;
		gbnd[1] += xx1;
		gbnd[2] -= yy1;
		gbnd[3] += yy1;
                }

	/* if bounds not specified then quit */
	if (gbnd[0] >= gbnd[1] || gbnd[2] >= gbnd[3])
		{
		fprintf(outfp,"\nGrid bounds not properly specified:\n\t%f %f %f %f\n",gbnd[0],gbnd[1],gbnd[2],gbnd[3]);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_PARAMETER;
		exit(error);
		}

	/* footprint option only for bathymetry */
	if ((grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE || grid_mode == MBGRID_WEIGHTED_FOOTPRINT)
		&& (datatype != MBGRID_DATA_TOPOGRAPHY && datatype != MBGRID_DATA_BATHYMETRY))
		{
		grid_mode = MBGRID_WEIGHTED_MEAN;
		}

	/* more option not available with minimum
		or maximum filter algorithms */
	if (more == MB_YES
		&& (grid_mode == MBGRID_MINIMUM_FILTER
		    || grid_mode == MBGRID_MAXIMUM_FILTER))
		more = MB_NO;

	/* NaN cannot be used for ASCII grids */
	if (use_NaN == MB_YES
		&& (gridkind == MBGRID_ASCII
		    || gridkind == MBGRID_ARCASCII))
		use_NaN = MB_NO;

	/* define NaN in case it's needed */
	if (use_NaN == MB_YES)
		{
		GMT_make_fnan(NaN);
		outclipvalue = NaN;
		}

	/* deal with projected gridding */
	if (projection_pars_f == MB_YES)
		{
		/* check for UTM with undefined zone */
		if (strcmp(projection_pars, "UTM") == 0
			|| strcmp(projection_pars, "U") == 0
			|| strcmp(projection_pars, "utm") == 0
			|| strcmp(projection_pars, "u") == 0)
			{
			reference_lon = 0.5 * (gbnd[0] + gbnd[1]);
			if (reference_lon < 180.0)
				reference_lon += 360.0;
			if (reference_lon >= 180.0)
				reference_lon -= 360.0;
			utm_zone = (int)(((reference_lon + 183.0)
				/ 6.0) + 0.5);
			reference_lat = 0.5 * (gbnd[2] + gbnd[3]);
			if (reference_lat >= 0.0)
				sprintf(projection_id, "UTM%2.2dN", utm_zone);
			else
				sprintf(projection_id, "UTM%2.2dS", utm_zone);
			}
		else
			strcpy(projection_id, projection_pars);

		/* set projection flag */
		use_projection = MB_YES;
		proj_status = mb_proj_init(verbose,projection_id,
			&(pjptr), &error);

		/* if projection not successfully initialized then quit */
		if (proj_status != MB_SUCCESS)
			{
			fprintf(outfp,"\nOutput projection %s not found in database\n",
				projection_id);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			error = MB_ERROR_BAD_PARAMETER;
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		/* tranlate lon lat bounds from UTM if required */
		if (gbnd[0] < -360.0 || gbnd[0] > 360.0
			|| gbnd[1] < -360.0 || gbnd[1] > 360.0
			|| gbnd[2] < -90.0 || gbnd[2] > 90.0
			|| gbnd[3] < -90.0 || gbnd[3] > 90.0)
			{
			/* first point */
			xx = gbnd[0];
			yy = gbnd[2];
			mb_proj_inverse(verbose, pjptr, xx, yy,
					&xlon, &ylat, &error);
			mb_apply_lonflip(verbose, lonflip, &xlon);
			obnd[0] = xlon;
			obnd[1] = xlon;
			obnd[2] = ylat;
			obnd[3] = ylat;

			/* second point */
			xx = gbnd[1];
			yy = gbnd[2];
			mb_proj_inverse(verbose, pjptr, xx, yy,
					&xlon, &ylat, &error);
			mb_apply_lonflip(verbose, lonflip, &xlon);
			obnd[0] = MIN(obnd[0], xlon);
			obnd[1] = MAX(obnd[1], xlon);
			obnd[2] = MIN(obnd[2], ylat);
			obnd[3] = MAX(obnd[3], ylat);

			/* third point */
			xx = gbnd[0];
			yy = gbnd[3];
			mb_proj_inverse(verbose, pjptr, xx, yy,
					&xlon, &ylat, &error);
			mb_apply_lonflip(verbose, lonflip, &xlon);
			obnd[0] = MIN(obnd[0], xlon);
			obnd[1] = MAX(obnd[1], xlon);
			obnd[2] = MIN(obnd[2], ylat);
			obnd[3] = MAX(obnd[3], ylat);

			/* fourth point */
			xx = gbnd[1];
			yy = gbnd[3];
			mb_proj_inverse(verbose, pjptr, xx, yy,
					&xlon, &ylat, &error);
			mb_apply_lonflip(verbose, lonflip, &xlon);
			obnd[0] = MIN(obnd[0], xlon);
			obnd[1] = MAX(obnd[1], xlon);
			obnd[2] = MIN(obnd[2], ylat);
			obnd[3] = MAX(obnd[3], ylat);
			}

		/* else translate bounds to UTM */
		else
			{
			/* copy gbnd to obnd */
			obnd[0] = gbnd[0];
			obnd[1] = gbnd[1];
			obnd[2] = gbnd[2];
			obnd[3] = gbnd[3];

			/* first point */
			xlon = obnd[0];
			ylat = obnd[2];
			mb_proj_forward(verbose, pjptr, xlon, ylat,
					&xx, &yy, &error);
			gbnd[0] = xx;
			gbnd[1] = xx;
			gbnd[2] = yy;
			gbnd[3] = yy;

			/* second point */
			xlon = obnd[1];
			ylat = obnd[2];
			mb_proj_forward(verbose, pjptr, xlon, ylat,
					&xx, &yy, &error);
			gbnd[0] = MIN(gbnd[0], xx);
			gbnd[1] = MAX(gbnd[1], xx);
			gbnd[2] = MIN(gbnd[2], yy);
			gbnd[3] = MAX(gbnd[3], yy);

			/* third point */
			xlon = obnd[0];
			ylat = obnd[3];
			mb_proj_forward(verbose, pjptr, xlon, ylat,
					&xx, &yy, &error);
			gbnd[0] = MIN(gbnd[0], xx);
			gbnd[1] = MAX(gbnd[1], xx);
			gbnd[2] = MIN(gbnd[2], yy);
			gbnd[3] = MAX(gbnd[3], yy);

			/* fourth point */
			xlon = obnd[1];
			ylat = obnd[3];
			mb_proj_forward(verbose, pjptr, xlon, ylat,
					&xx, &yy, &error);
			gbnd[0] = MIN(gbnd[0], xx);
			gbnd[1] = MAX(gbnd[1], xx);
			gbnd[2] = MIN(gbnd[2], yy);
			gbnd[3] = MAX(gbnd[3], yy);
			}

		/* calculate grid properties */
		if (set_spacing == MB_YES)
			{
			xdim = (gbnd[1] - gbnd[0])/dx_set + 1;
			if (dy_set <= 0.0)
				dy_set = dx_set;
			ydim = (gbnd[3] - gbnd[2])/dy_set + 1;
			if (spacing_priority == MB_YES)
				{
				gbnd[1] = gbnd[0] + dx_set * (xdim - 1);
				gbnd[3] = gbnd[2] + dy_set * (ydim - 1);
				}
			if (units[0] == 'M' || units[0] == 'm')
				strcpy(units, "meters");
			else if (units[0] == 'K' || units[0] == 'k')
				strcpy(units, "km");
			else if (units[0] == 'F' || units[0] == 'f')
				strcpy(units, "feet");
			else
				strcpy(units, "unknown");
			}

/* fprintf(outfp," Projected coordinates on: proj_status:%d  projection:%s\n",
proj_status, projection_id);
fprintf(outfp," Lon Lat Bounds: %f %f %f %f\n",
obnd[0], obnd[1], obnd[2], obnd[3]);
fprintf(outfp," XY Bounds: %f %f %f %f\n",
gbnd[0], gbnd[1], gbnd[2], gbnd[3]);*/

		}

	/* deal with no projection */
	else
		{

		/* calculate grid properties */
		mb_coor_scale(verbose,0.5*(gbnd[2]+gbnd[3]),&mtodeglon,&mtodeglat);
		deglontokm = 0.001/mtodeglon;
		deglattokm = 0.001/mtodeglat;
		if (set_spacing == MB_YES
			&& (units[0] == 'M' || units[0] == 'm'))
			{
			xdim = (gbnd[1] - gbnd[0])/(mtodeglon*dx_set) + 1;
			if (dy_set <= 0.0)
				dy_set = mtodeglon * dx_set / mtodeglat;
			ydim = (gbnd[3] - gbnd[2])/(mtodeglat*dy_set) + 1;
			if (spacing_priority == MB_YES)
				{
				gbnd[1] = gbnd[0] + mtodeglon * dx_set * (xdim - 1);
				gbnd[3] = gbnd[2] + mtodeglat * dy_set * (ydim - 1);
				}
			strcpy(units, "meters");
			}
		else if (set_spacing == MB_YES
			&& (units[0] == 'K' || units[0] == 'k'))
			{
			xdim = (gbnd[1] - gbnd[0])*deglontokm/dx_set + 1;
			if (dy_set <= 0.0)
				dy_set = deglattokm * dx_set / deglontokm;
			ydim = (gbnd[3] - gbnd[2])*deglattokm/dy_set + 1;
			if (spacing_priority == MB_YES)
				{
				gbnd[1] = gbnd[0] + dx_set * (xdim - 1) / deglontokm;
				gbnd[3] = gbnd[2] + dy_set * (ydim - 1) / deglattokm;
				}
			strcpy(units, "km");
			}
		else if (set_spacing == MB_YES
			&& (units[0] == 'F' || units[0] == 'f'))
			{
			xdim = (gbnd[1] - gbnd[0])/(mtodeglon*0.3048*dx_set) + 1;
			if (dy_set <= 0.0)
				dy_set = mtodeglon * dx_set / mtodeglat;
			ydim = (gbnd[3] - gbnd[2])/(mtodeglat*0.3048*dy_set) + 1;
			if (spacing_priority == MB_YES)
				{
				gbnd[1] = gbnd[0] + mtodeglon * 0.3048 * dx_set * (xdim - 1);
				gbnd[3] = gbnd[2] + mtodeglat * 0.3048 * dy_set * (ydim - 1);
				}
			strcpy(units, "feet");
			}
		else if (set_spacing == MB_YES)
			{
			xdim = (gbnd[1] - gbnd[0])/dx_set + 1;
			if (dy_set <= 0.0)
				dy_set = dx_set;
			ydim = (gbnd[3] - gbnd[2])/dy_set + 1;
			if (spacing_priority == MB_YES)
				{
				gbnd[1] = gbnd[0] + dx_set * (xdim - 1);
				gbnd[3] = gbnd[2] + dy_set * (ydim - 1);
				}
			strcpy(units, "degrees");
			}
		}

	/* calculate other grid properties */
	dx = (gbnd[1] - gbnd[0])/(xdim-1);
	dy = (gbnd[3] - gbnd[2])/(ydim-1);
	factor = 4.0/(scale*scale*dx*dy);
	offx = 0;
	offy = 0;
	if (extend > 0.0)
		{
		offx = (int) (extend*xdim);
		offy = (int) (extend*ydim);
		}
	xtradim = scale + 2;
	gxdim = xdim + 2*offx;
	gydim = ydim + 2*offy;
	wbnd[0] = gbnd[0] - offx*dx;
	wbnd[1] = gbnd[1] + offx*dx;
	wbnd[2] = gbnd[2] - offy*dy;
	wbnd[3] = gbnd[3] + offy*dy;
	if (datatype == MBGRID_DATA_TOPOGRAPHY)
		topofactor = -1.0;
	else
		topofactor = 1.0;
	if (bathy_in_feet == MB_YES
		&& (datatype == MBGRID_DATA_TOPOGRAPHY
		|| datatype == MBGRID_DATA_BATHYMETRY))
		topofactor = topofactor / 0.3048;

	/* check that dx == dy for Arc ascii grid output */
	if (gridkind == MBGRID_ARCASCII && fabs(dx - dy) > MBGRID_TINY)
		{
		fprintf(outfp,"\nArc Ascii grid output (-G4) requires square cells, but grid intervals dx:%f dy:%f differ...\n", dx, dy);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_PARAMETER;
		exit(error);
		}

	/* get data input bounds in lon lat */
	if (use_projection == MB_NO)
		{
		bounds[0] = wbnd[0];
		bounds[1] = wbnd[1];
		bounds[2] = wbnd[2];
		bounds[3] = wbnd[3];
		}
	/* get min max of lon lat for data input from projected bounds */
	else
		{
		/* do first point */
		xx = wbnd[0] - (wbnd[1] - wbnd[0]);
		yy = wbnd[2] - (wbnd[3] - wbnd[2]);
		mb_proj_inverse(verbose, pjptr,
					xx, yy,
					&xlon, &ylat,
					&error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = xlon;
		bounds[1] = xlon;
		bounds[2] = ylat;
		bounds[3] = ylat;

		/* do second point */
		xx = wbnd[0] + (wbnd[1] - wbnd[0]);
		yy = wbnd[2] - (wbnd[3] - wbnd[2]);
		mb_proj_inverse(verbose, pjptr,
					xx, yy,
					&xlon, &ylat,
					&error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = MIN(bounds[0], xlon);
		bounds[1] = MAX(bounds[1], xlon);
		bounds[2] = MIN(bounds[2], ylat);
		bounds[3] = MAX(bounds[3], ylat);

		/* do third point */
		xx = wbnd[0] - (wbnd[1] - wbnd[0]);
		yy = wbnd[2] + (wbnd[3] - wbnd[2]);
		mb_proj_inverse(verbose, pjptr,
					xx, yy,
					&xlon, &ylat,
					&error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = MIN(bounds[0], xlon);
		bounds[1] = MAX(bounds[1], xlon);
		bounds[2] = MIN(bounds[2], ylat);
		bounds[3] = MAX(bounds[3], ylat);

		/* do fourth point */
		xx = wbnd[0] + (wbnd[1] - wbnd[0]);
		yy = wbnd[2] + (wbnd[3] - wbnd[2]);
		mb_proj_inverse(verbose, pjptr,
					xx, yy,
					&xlon, &ylat,
					&error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = MIN(bounds[0], xlon);
		bounds[1] = MAX(bounds[1], xlon);
		bounds[2] = MIN(bounds[2], ylat);
		bounds[3] = MAX(bounds[3], ylat);
		}

	/* extend the bounds slightly to be sure no data gets missed */
	xx = MIN(0.05*(bounds[1] - bounds[0]), 0.1);
	yy = MIN(0.05*(bounds[3] - bounds[2]), 0.1);
	bounds[0] = bounds[0] - xx;
	bounds[1] = bounds[1] + xx;
	bounds[2] = bounds[2] - yy;
	bounds[3] = bounds[3] + yy;

	/* figure out lonflip for data bounds */
	if (bounds[0] < -180.0)
		lonflip = -1;
	else if (bounds[1] > 180.0)
		lonflip = 1;
	else if (lonflip == -1 && bounds[1] > 0.0)
		lonflip = 0;
	else if (lonflip == 1 && bounds[0] < 0.0)
		lonflip = 0;

	/* check interpolation parameters */
	if ((clipmode == MBGRID_INTERP_GAP
		|| clipmode == MBGRID_INTERP_NEAR)
		&& clip > xdim && clip > ydim)
		clipmode = MBGRID_INTERP_ALL;
        if (clipmode == MBGRID_INTERP_ALL)
                clip = MAX(xdim, ydim);
        
        /* set origin used to reduce data value size before conversion from
         * double to float when calling the interpolation routines */
        bdata_origin_x = 0.5 * (wbnd[0] + wbnd[1]);
        bdata_origin_y = 0.5 * (wbnd[2] + wbnd[3]);

	/* set plot label strings */
	if (use_projection == MB_YES)
		{
		sprintf(xlabel,"Easting (%s)", units);
		sprintf(ylabel,"Northing (%s)", units);
		}
	else
		{
		strcpy(xlabel,"Longitude");
		strcpy(ylabel,"Latitude");
		}
	if (datatype == MBGRID_DATA_BATHYMETRY)
		{
		if (bathy_in_feet == MB_YES)
			strcpy(zlabel,"Depth (ft)");
		else
			strcpy(zlabel,"Depth (m)");
		strcpy(nlabel,"Number of Depth Data Points");
		if (bathy_in_feet == MB_YES)
			strcpy(sdlabel,"Depth Standard Deviation (ft)");
		else
			strcpy(sdlabel,"Depth Standard Deviation (m)");
		strcpy(title,"Bathymetry Grid");
		}
	else if (datatype == MBGRID_DATA_TOPOGRAPHY)
		{
		if (bathy_in_feet == MB_YES)
			strcpy(zlabel,"Topography (ft)");
		else
			strcpy(zlabel,"Topography (m)");
		strcpy(nlabel,"Number of Topography Data Points");
		if (bathy_in_feet == MB_YES)
			strcpy(sdlabel,"Topography Standard Deviation (ft)");
		else
			strcpy(sdlabel,"Topography Standard Deviation (m)");
		strcpy(title,"Topography Grid");
		}
	else if (datatype == MBGRID_DATA_AMPLITUDE)
		{
		strcpy(zlabel,"Amplitude");
		strcpy(nlabel,"Number of Amplitude Data Points");
		strcpy(sdlabel,"Amplitude Standard Deviation (m)");
		strcpy(title,"Amplitude Grid");
		}
	else if (datatype == MBGRID_DATA_SIDESCAN)
		{
		strcpy(zlabel,"Sidescan");
		strcpy(nlabel,"Number of Sidescan Data Points");
		strcpy(sdlabel,"Sidescan Standard Deviation (m)");
		strcpy(title,"Sidescan Grid");
		}

	/* output info */
	if (verbose >= 0)
		{
		fprintf(outfp,"\nMBGRID Parameters:\n");
		fprintf(outfp,"List of input files: %s\n",filelist);
		fprintf(outfp,"Output fileroot:     %s\n",fileroot);
		fprintf(outfp,"Input Data Type:     ");
		if (datatype == MBGRID_DATA_BATHYMETRY)
			{
			fprintf(outfp,"Bathymetry\n");
			if (bathy_in_feet == MB_YES)
				fprintf(outfp,"Bathymetry gridded in feet\n");
			}
		else if (datatype == MBGRID_DATA_TOPOGRAPHY)
			{
			fprintf(outfp,"Topography\n");
			if (bathy_in_feet == MB_YES)
				fprintf(outfp,"Topography gridded in feet\n");
			}
		else if (datatype == MBGRID_DATA_AMPLITUDE)
			fprintf(outfp,"Amplitude\n");
		else if (datatype == MBGRID_DATA_SIDESCAN)
			fprintf(outfp,"Sidescan\n");
		else
			fprintf(outfp,"Unknown?\n");
		fprintf(outfp,"Gridding algorithm:  ");
		if (grid_mode == MBGRID_MEDIAN_FILTER)
			fprintf(outfp,"Median Filter\n");
		else if (grid_mode == MBGRID_MINIMUM_FILTER)
			fprintf(outfp,"Minimum Filter\n");
		else if (grid_mode == MBGRID_MAXIMUM_FILTER)
			fprintf(outfp,"Maximum Filter\n");
		else if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE)
			fprintf(outfp,"Footprint-Slope Weighted Mean\n");
		else if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT)
			fprintf(outfp,"Footprint Weighted Mean\n");
		else
			fprintf(outfp,"Gaussian Weighted Mean\n");
		fprintf(outfp,"Grid projection: %s\n", projection_id);
		if (use_projection == MB_YES)
			{
			fprintf(outfp,"Projection ID: %s\n", projection_id);
			}
		fprintf(outfp,"Grid dimensions: %d %d\n",xdim,ydim);
		fprintf(outfp,"Grid bounds:\n");
		if (use_projection == MB_YES)
			{
			fprintf(outfp,"  Eastings:  %9.4f %9.4f\n",gbnd[0],gbnd[1]);
			fprintf(outfp,"  Northings: %9.4f %9.4f\n",gbnd[2],gbnd[3]);
			fprintf(outfp,"  Longitude: %9.4f %9.4f\n",obnd[0],obnd[1]);
			fprintf(outfp,"  Latitude:  %9.4f %9.4f\n",obnd[2],obnd[3]);
			}
		else
			{
			fprintf(outfp,"  Longitude: %9.4f %9.4f\n",gbnd[0],gbnd[1]);
			fprintf(outfp,"  Latitude:  %9.4f %9.4f\n",gbnd[2],gbnd[3]);
			}
                if (boundsfactor > 1.0)
                    fprintf(outfp,"  Grid bounds correspond to %f times actual data coverage\n",boundsfactor);
 		fprintf(outfp,"Working grid dimensions: %d %d\n",gxdim,gydim);
		if (use_projection == MB_YES)
			{
			fprintf(outfp,"Working Grid bounds:\n");
			fprintf(outfp,"  Eastings:  %9.4f %9.4f\n",wbnd[0],wbnd[1]);
			fprintf(outfp,"  Northings: %9.4f %9.4f\n",wbnd[2],wbnd[3]);
			fprintf(outfp,"Easting interval:  %f %s\n",
				dx,units);
			fprintf(outfp,"Northing interval: %f %s\n",
				dy,units);
			if (set_spacing == MB_YES)
				{
				fprintf(outfp,"Specified Easting interval:  %f %s\n",
					dx_set, units);
				fprintf(outfp,"Specified Northing interval: %f %s\n",
					dy_set, units);
				}
			}
		else
			{
			fprintf(outfp,"Working Grid bounds:\n");
			fprintf(outfp,"  Longitude: %9.4f %9.4f\n",wbnd[0],wbnd[1]);
			fprintf(outfp,"  Latitude:  %9.4f %9.4f\n",wbnd[2],wbnd[3]);
			fprintf(outfp,"Longitude interval: %f degrees or %f m\n",
				dx,1000*dx*deglontokm);
			fprintf(outfp,"Latitude interval:  %f degrees or %f m\n",
				dy,1000*dy*deglattokm);
			if (set_spacing == MB_YES)
				{
				fprintf(outfp,"Specified Longitude interval: %f %s\n",
					dx_set, units);
				fprintf(outfp,"Specified Latitude interval:  %f %s\n",
					dy_set, units);
				}
			}
		fprintf(outfp,"Input data bounds:\n");
		fprintf(outfp,"  Longitude: %9.4f %9.4f\n",bounds[0],bounds[1]);
		fprintf(outfp,"  Latitude:  %9.4f %9.4f\n",bounds[2],bounds[3]);
		if (grid_mode == MBGRID_WEIGHTED_MEAN)
			fprintf(outfp,"Gaussian filter 1/e length: %f grid intervals\n",
				scale);
		if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE
                        || grid_mode == MBGRID_WEIGHTED_FOOTPRINT)
			fprintf(outfp,"Footprint 1/e distance: %f times footprint\n",
				scale);
		if (check_time == MB_YES && first_in_stays == MB_NO)
			fprintf(outfp,"Swath overlap handling:       Last data used\n");
		if (check_time == MB_YES && first_in_stays == MB_YES)
			fprintf(outfp,"Swath overlap handling:       First data used\n");
		if (check_time == MB_YES)
			fprintf(outfp,"Swath overlap time threshold: %f minutes\n",
				timediff/60.);
		if (clipmode == MBGRID_INTERP_NONE)
			fprintf(outfp,"Spline interpolation not applied\n");
		else if (clipmode == MBGRID_INTERP_GAP)
			{
			fprintf(outfp,"Spline interpolation applied to fill data gaps\n");
			fprintf(outfp,"Spline interpolation clipping dimension: %d\n",clip);
			fprintf(outfp,"Spline tension (range 0.0 to infinity): %f\n",tension);
			}
		else if (clipmode == MBGRID_INTERP_NEAR)
			{
			fprintf(outfp,"Spline interpolation applied near data\n");
			fprintf(outfp,"Spline interpolation clipping dimension: %d\n",clip);
			fprintf(outfp,"Spline tension (range 0.0 to infinity): %f\n",tension);
			}
		else if (clipmode == MBGRID_INTERP_ALL)
			{
			fprintf(outfp,"Spline interpolation applied to fill entire grid\n");
			fprintf(outfp,"Spline tension (range 0.0 to infinity): %f\n",tension);
			}
		if (grdrasterid == 0)
			fprintf(outfp,"Background not applied\n");
		else if (grdrasterid < 0)
			fprintf(outfp,"Background obtained using grd2xyz from GMT grid file: %s\n",backgroundfile);
		else
			fprintf(outfp,"Background obtained using grdraster from dataset: %d\n",grdrasterid);
		if (gridkind == MBGRID_ASCII)
			fprintf(outfp,"Grid format %d:  ascii table\n",gridkind);
		else if (gridkind == MBGRID_CDFGRD)
			fprintf(outfp,"Grid format %d:  GMT version 2 grd (netCDF)\n",gridkind);
		else if (gridkind == MBGRID_OLDGRD)
			fprintf(outfp,"Grid format %d:  GMT version 1 grd (binary)\n",gridkind);
		else if (gridkind == MBGRID_ARCASCII)
			fprintf(outfp,"Grid format %d:  Arc/Info ascii table\n",gridkind);
		else if (gridkind == MBGRID_GMTGRD)
			{
			fprintf(outfp,"Grid format %d:  GMT grid\n",gridkind);
			if (strlen(gridkindstring) > 0)
				fprintf(outfp,"GMT Grid ID:     %s\n",gridkindstring);
			}
		if (use_NaN == MB_YES)
			fprintf(outfp,"NaN values used to flag regions with no data\n");
		else
			fprintf(outfp,"Real value of %f used to flag regions with no data\n",
				outclipvalue);
		if (more == MB_YES)
			fprintf(outfp,"Data density and sigma grids also created\n");
		fprintf(outfp,"MBIO parameters:\n");
		fprintf(outfp,"  Ping averaging:       %d\n",pings);
		fprintf(outfp,"  Longitude flipping:   %d\n",lonflip);
		fprintf(outfp,"  Speed minimum:      %4.1f km/hr\n",speedmin);
		}
	if (verbose > 0)
		fprintf(outfp,"\n");

	/* if grdrasterid set extract background data
		and interpolate it later onto internal grid */
	if (grdrasterid != 0)
		{
		if (grdrasterid > 0)
			fprintf(outfp,"\nExtracting background from grdraster dataset %d...\n",grdrasterid);
		else
			fprintf(outfp,"\nExtracting background from grid file %s...\n",backgroundfile);

		/* guess about twice the data actually expected */
		if (use_projection == MB_YES)
			nbackground_alloc = 2 * gxdim * gydim;
		else
			nbackground_alloc = 2 * gxdim * gydim;

		/* allocate and initialize background data arrays */
#ifdef USESURFACE
		status = mb_mallocd(verbose,__FILE__,__LINE__,nbackground_alloc*sizeof(float),(void **)&bxdata,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,nbackground_alloc*sizeof(float),(void **)&bydata,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,nbackground_alloc*sizeof(float),(void **)&bzdata,&error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
			fprintf(outfp,"\nMBIO Error allocating background data array:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		memset((char *)bxdata,0,nbackground_alloc*sizeof(float));
		memset((char *)bydata,0,nbackground_alloc*sizeof(float));
		memset((char *)bzdata,0,nbackground_alloc*sizeof(float));
#else
		status = mb_mallocd(verbose,__FILE__,__LINE__,3*nbackground_alloc*sizeof(float),(void **)&bdata,&error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
			fprintf(outfp,"\nMBIO Error allocating background interpolation work arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		memset((char *)bdata,0,3*nbackground_alloc*sizeof(float));
#endif

		/* get initial grid using grdraster */
		if (grdrasterid > 0)
			{
			sprintf(backgroundfile,"tmpgrdraster%d.grd", pid);
			sprintf(plot_cmd, "grdraster %d -R%f/%f/%f/%f -G%s",
				grdrasterid,bounds[0],bounds[1],bounds[2],bounds[3],backgroundfile);
			fprintf(stderr, "Executing: %s\n", plot_cmd);
			fork_status = system(plot_cmd);
			if (fork_status != 0)
				{
				fprintf(outfp,"\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n",
						plot_cmd,program_name);
				error = MB_ERROR_BAD_PARAMETER;
				mb_memory_clear(verbose, &error);
				exit(error);
				}
			}

		/* if needed translate grid to normal registration */
		sprintf(plot_cmd, "grdinfo %s", backgroundfile);
		strcpy(backgroundfileuse, backgroundfile);
		if ((rfp = popen(plot_cmd,"r")) != NULL)
			{
			/* parse the grdinfo results */
			bufptr = fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp);
			bufptr = fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp);
			bufptr = fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp);
			bufptr = fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp);
			pclose(rfp);
			if (strncmp(plot_stdout,"Pixel node registration used", 28) == 0)
				{
				sprintf(backgroundfileuse, "tmpgrdsampleT%d.grd", pid);
				sprintf(plot_cmd, "grdsample %s -G%s -T",
					backgroundfile, backgroundfileuse);
				fprintf(stderr, "Executing: %s\n", plot_cmd);
				fork_status = system(plot_cmd);
				if (fork_status != 0)
					{
					fprintf(outfp,"\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n",
							plot_cmd,program_name);
					error = MB_ERROR_BAD_PARAMETER;
					mb_memory_clear(verbose, &error);
					exit(error);
					}
				}
			}
		else
			{
			fprintf(outfp,"\nBackground data not extracted as per -K option\n");
			if (grdrasterid > 0)
				{
				fprintf(outfp,"The program grdraster may not have been found\n");
				fprintf(outfp,"or the specified background dataset %d may not exist.\n",
					grdrasterid);
				}
			else
				{
				fprintf(outfp,"The specified background dataset %s may not exist.\n",
					backgroundfile);
				}
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			error = MB_ERROR_BAD_PARAMETER;
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		/* resample extracted grid to have similar resolution as working grid */
		sprintf(plot_cmd, "grdsample %s -Gtmpgrdsample%d.grd -R%.12f/%.12f/%.12f/%.12f -I%.12f/%.12f",
				backgroundfileuse, pid,bounds[0],bounds[1],bounds[2],bounds[3], dx, dy);
		fprintf(stderr, "Executing: %s\n", plot_cmd);
		fork_status = system(plot_cmd);
		if (fork_status != 0)
			{
			fprintf(outfp,"\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n",
					plot_cmd,program_name);
			error = MB_ERROR_BAD_PARAMETER;
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		/* extract points with preprocessing if that will help */
		if (use_projection == MB_NO)
			{
			sprintf(plot_cmd, "grd2xyz tmpgrdsample%d.grd -S -bo | blockmean -bi -bo -C -R%f/%f/%f/%f -I%.12f/%.12f",
				pid, bounds[0], bounds[1], bounds[2], bounds[3], dx, dy);
			}
		else
			{
			sprintf(plot_cmd, "grd2xyz tmpgrdsample%d.grd -S -bo",
				pid);
			}
		fprintf(stderr, "Executing: %s\n", plot_cmd);
		if ((rfp = popen(plot_cmd,"r")) != NULL)
			{
			/* loop over reading */
			nbackground = 0;
			while (fread(&tlon, sizeof(double), 1, rfp) == 1)
				{
				freadsize = fread(&tlat, sizeof(double), 1, rfp);
				freadsize = fread(&tvalue, sizeof(double), 1, rfp);
				if (lonflip == -1 && tlon > 0.0)
					tlon -= 360.0;
				else if (lonflip == 0 && tlon < -180.0)
					tlon += 360.0;
				else if (lonflip == 0 && tlon > 180.0)
					tlon -= 360.0;
				else if (lonflip == 1 && tlon < 0.0)
					tlon += 360.0;
				if (use_projection == MB_YES)
					mb_proj_forward(verbose, pjptr, tlon, tlat,
					&tlon, &tlat, &error);
#ifdef USESURFACE
				if (nbackground >= nbackground_alloc)
					{
					nbackground_alloc += 10000;
					status = mb_reallocd(verbose,__FILE__,__LINE__,nbackground_alloc*sizeof(float),(void **)&bxdata,&error);
					if (status == MB_SUCCESS)
						status = mb_reallocd(verbose,__FILE__,__LINE__,nbackground_alloc*sizeof(float),(void **)&bydata,&error);
					if (status == MB_SUCCESS)
						status = mb_reallocd(verbose,__FILE__,__LINE__,nbackground_alloc*sizeof(float),(void **)&bzdata,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
						fprintf(outfp,"\nMBIO Error reallocating background data array:\n%s\n",
							message);
						fprintf(outfp,"\nProgram <%s> Terminated\n",
							program_name);
						mb_memory_clear(verbose, &error);
						exit(error);
						}
					}
				bxdata[nbackground] = (float) (tlon - bdata_origin_x);
				bydata[nbackground] = (float) (tlat - bdata_origin_y);
				bzdata[nbackground] = (float) tvalue;
#else
				if (nbackground >= nbackground_alloc)
					{
					nbackground_alloc += 10000;
					status = mb_reallocd(verbose,__FILE__,__LINE__,3*nbackground_alloc*sizeof(float),(void **)&bdata,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
						fprintf(outfp,"\nMBIO Error allocating background interpolation work arrays:\n%s\n",
							message);
						fprintf(outfp,"\nProgram <%s> Terminated\n",
							program_name);
						mb_memory_clear(verbose, &error);
						exit(error);
						}
					}
				bdata[nbackground*3] = (float) (tlon - bdata_origin_x);
				bdata[nbackground*3+1] = (float) (tlat - bdata_origin_y);
				bdata[nbackground*3+2] = (float) tvalue;
#endif
				nbackground++;
				}
			pclose(rfp);
			}
		else
			{
			fprintf(outfp,"\nBackground data not extracted as per -K option\n");
			fprintf(outfp,"The program grdraster may not have been found\n");
			fprintf(outfp,"or the specified background dataset %d may not exist.\n",
				grdrasterid);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			error = MB_ERROR_BAD_PARAMETER;
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		/* delete any temporary files */
		sprintf(plot_cmd, "rm tmpgrd*%d.grd", pid);
		fprintf(stderr, "Executing: %s\n", plot_cmd);
		fork_status = system(plot_cmd);
		if (fork_status != 0)
			{
			fprintf(outfp,"\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n",
					plot_cmd,program_name);
			error = MB_ERROR_BAD_PARAMETER;
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		}

	/* allocate memory for grid arrays */
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(double),(void **)&grid,&error);
	if (status == MB_SUCCESS)
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(double),(void **)&sigma,&error);
	if (status == MB_SUCCESS)
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(double),(void **)&firsttime,&error);
	if (status == MB_SUCCESS)
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(int),(void **)&cnt,&error);
	if (status == MB_SUCCESS)
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(int),(void **)&num,&error);
	if (status == MB_SUCCESS)
	status = mb_mallocd(verbose,__FILE__,__LINE__,xdim*ydim*sizeof(float),(void **)&output,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}

	/* open datalist file for list of all files that contribute to the grid */
	strcpy(dfile,fileroot);
	strcat(dfile,".mb-1");
	if ((dfp = fopen(dfile,"w")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open datalist file: %s\n",
			dfile);
		}

	/***** do weighted footprint slope gridding *****/
	if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE)
	{
	/* set up parameters for first cut low resolution slope grid */
	for (i=0;i<4;i++)
		sbnd[i] = wbnd[i];
	sdx = 2.0 * dx;
	sdy = 2.0 * dy;
	sxdim = gxdim  / 2;
	sydim = gydim  / 2;
	sclip = MAX(gxdim, gydim);

	/* allocate memory for additional arrays */
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(double),(void **)&norm,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,sxdim*sydim*sizeof(double),(void **)&gridsmall,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}

	/* do first pass using simple mean to get low-resolution quick bathymetry to provide bottom slope
		estimates for footprint gridding */

	/* initialize arrays */
	for (i=0;i<sxdim;i++)
		for (j=0;j<sydim;j++)
			{
			kgrid = i * sydim + j;
			gridsmall[kgrid] = 0.0;
			cnt[kgrid] = 0;
			}

	/* read in data */
	fprintf(outfp,"\nDoing first pass to generate low resolution slope grid...\n");
	ndata = 0;
	if ((status = mb_datalist_open(verbose,&datalist,
					filelist,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}
	while ((status = mb_datalist_read2(verbose,datalist,
			&pstatus,path,ppath,&format,&file_weight,&error))
			== MB_SUCCESS)
		{
		ndatafile = 0;

		/* if format > 0 then input is swath sonar file */
		if (format > 0 && path[0] != '#')
		{
		/* apply pstatus */
		if (pstatus == MB_PROCESSED_USE)
			strcpy(file, ppath);
		else
			strcpy(file, path);

		/* check for mbinfo file - get file bounds if possible */
		rformat = format;
		strcpy(rfile,file);
		status = mb_check_info(verbose, rfile, lonflip, bounds,
				&file_in_bounds, &error);
		if (status == MB_FAILURE)
			{
			file_in_bounds = MB_YES;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* initialize the swath sonar file */
		if (file_in_bounds == MB_YES)
		    {
		    /* check for "fast bathymetry" or "fbt" file */
		    if (datatype == MBGRID_DATA_TOPOGRAPHY
			    || datatype == MBGRID_DATA_BATHYMETRY)
			{
			mb_get_fbt(verbose, rfile, &rformat, &error);
			}

		    /* call mb_read_init() */
		    if ((status = mb_read_init(
			verbose,rfile,rformat,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&mbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
			fprintf(outfp,"\nMultibeam File <%s> not initialized for reading\n",rfile);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		    /* get mb_io_ptr */
		    mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
                    
                    /* get topography type */
                    status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);

		    /* allocate memory for reading data arrays */
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(char), (void **)&beamflag, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bath, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&amp, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathlon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathlat, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&ss, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslat, &error);

		    /* if error initializing memory then quit */
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		    /* loop over reading */
		    while (error <= MB_ERROR_NO_ERROR)
			{
			status = mb_read(verbose,mbio_ptr,&kind,
				&rpings,time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&beams_bath,&beams_amp,&pixels_ss,
				beamflag,bath,amp,bathlon,bathlat,
				ss,sslon,sslat,
				comment,&error);

			/* time gaps are not a problem here */
			if (error == MB_ERROR_TIME_GAP)
				{
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
				}

			/* print debug statements */
			if (verbose >= 2)
				{
				fprintf(outfp,"\ndbg2  Ping read in program <%s>\n",program_name);
				fprintf(outfp,"dbg2       kind:           %d\n",kind);
				fprintf(outfp,"dbg2       beams_bath:     %d\n",beams_bath);
				fprintf(outfp,"dbg2       beams_amp:      %d\n",beams_amp);
				fprintf(outfp,"dbg2       pixels_ss:      %d\n",pixels_ss);
				fprintf(outfp,"dbg2       error:          %d\n",error);
				fprintf(outfp,"dbg2       status:         %d\n",status);
				}

			if ((datatype == MBGRID_DATA_BATHYMETRY
				|| datatype == MBGRID_DATA_TOPOGRAPHY)
				&& error == MB_ERROR_NO_ERROR)
			  {

			  /* reproject beam positions if necessary */
			  if (use_projection == MB_YES)
			    {
			    mb_proj_forward(verbose, pjptr,
					    navlon, navlat,
					    &navlon, &navlat,
					    &error);
			    for (ib=0;ib<beams_bath;ib++)
			      if (mb_beam_ok(beamflag[ib]))
				mb_proj_forward(verbose, pjptr,
						bathlon[ib], bathlat[ib],
						&bathlon[ib], &bathlat[ib],
						&error);
			    }

			  /* deal with data */
			  for (ib=0;ib<beams_bath;ib++)
			    if (mb_beam_ok(beamflag[ib]))
			      {
			      /* get position in grid */
			      ix = (bathlon[ib] - wbnd[0] + dx) / sdx;
			      iy = (bathlat[ib] - wbnd[2] + dy) / sdy;
/*fprintf(outfp, "\nib:%d ix:%d iy:%d   bath: lon:%f lat:%f bath:%f   nav: lon:%f lat:%f\n",
ib, ix, iy, bathlon[ib], bathlat[ib], bath[ib], navlon, navlat);*/

			      /* process if in region of interest */
			      if (ix >= 0
				&& ix < sxdim
				&& iy >= 0
				&& iy < sydim)
			        {
				kgrid = ix * sydim + iy;
				gridsmall[kgrid] += topofactor * bath[ib];
				cnt[kgrid]++;
				ndata++;
				ndatafile++;
				}
			      }
			  }
			}
		    status = mb_close(verbose,&mbio_ptr,&error);
		    status = MB_SUCCESS;
		    error = MB_ERROR_NO_ERROR;
		    }
		if (verbose >= 2)
			fprintf(outfp,"\n");
		if (verbose > 0 || file_in_bounds == MB_YES)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,rfile);
		} /* end if (format > 0) */

		}
	if (datalist != NULL)
		mb_datalist_close(verbose,&datalist,&error);
	if (verbose > 0)
		fprintf(outfp,"\n%d total data points processed\n",ndata);

	/* now loop over all points in the low resolution grid */
	if (verbose >= 1)
		fprintf(outfp,"\nMaking low resolution slope grid...\n");
	ndata = 8;
	for (i=0;i<sxdim;i++)
		for (j=0;j<sydim;j++)
			{
			kgrid = i * sydim + j;
			if (cnt[kgrid] > 0)
				{
				gridsmall[kgrid] = gridsmall[kgrid]/((double)cnt[kgrid]);
				ndata++;
				}
			}

	/* now fill in the low resolution grid with interpolation */
#ifdef USESURFACE
	/* allocate and initialize sgrid */
	status = mb_mallocd(verbose,__FILE__,__LINE__,ndata*sizeof(float),(void **)&sxdata,&error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__,__LINE__,ndata*sizeof(float),(void **)&sydata,&error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__,__LINE__,ndata*sizeof(float),(void **)&szdata,&error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__,__LINE__,sxdim*sydim*sizeof(float),(void **)&sgrid,&error);
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
		fprintf(outfp,"\nMBIO Error allocating interpolation work arrays:\n%s\n",
			message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}
	memset((char *)sgrid,0,sxdim*sydim*sizeof(float));
	memset((char *)sxdata,0,ndata*sizeof(float));
	memset((char *)sydata,0,ndata*sizeof(float));
	memset((char *)szdata,0,ndata*sizeof(float));

	/* get points from grid */
        /* simultaneously find the depth values nearest to the grid corners and edge midpoints */
	ndata = 0;
  	for (i=0;i<sxdim;i++)
		for (j=0;j<sydim;j++)
			{
			kgrid = i * sydim + j;
			if (cnt[kgrid] > 0)
				{
				sxdata[ndata] = (float)(wbnd[0] + sdx * i - bdata_origin_x);
				sydata[ndata] = (float)(wbnd[2] + sdy * j - bdata_origin_y);
				szdata[ndata] = (float)gridsmall[kgrid];
				ndata++;
 				}
			}

	/* do the interpolation */
	fprintf(outfp,"\nDoing Surface spline interpolation with %d data points...\n",ndata);
	mb_surface(verbose, ndata, sxdata, sydata, szdata,
		(wbnd[0] - bdata_origin_x), (wbnd[1] - bdata_origin_x),
                (wbnd[2] - bdata_origin_y), (wbnd[3] - bdata_origin_y),
                sdx, sdy,
		tension, sgrid);
#else
	/* allocate and initialize sgrid */
	status = mb_mallocd(verbose,__FILE__,__LINE__,3*ndata*sizeof(float),(void **)&sdata,&error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__,__LINE__,sxdim*sydim*sizeof(float),(void **)&sgrid,&error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__,__LINE__,ndata*sizeof(float),(void **)&work1,&error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__,__LINE__,ndata*sizeof(int),(void **)&work2,&error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__,__LINE__,(sxdim+sydim)*sizeof(int),(void **)&work3,&error);
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
		fprintf(outfp,"\nMBIO Error allocating interpolation work arrays:\n%s\n",
			message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}
	memset((char *)sgrid,0,sxdim*sydim*sizeof(float));
	memset((char *)sdata,0,3*ndata*sizeof(float));
	memset((char *)work1,0,ndata*sizeof(float));
	memset((char *)work2,0,ndata*sizeof(int));
	memset((char *)work3,0,(sxdim+sydim)*sizeof(int));

	/* get points from grid */
        /* simultaneously find the depth values nearest to the grid corners and edge midpoints */
	ndata = 0;
	for (i=0;i<sxdim;i++)
		for (j=0;j<sydim;j++)
			{
			kgrid = i * sydim + j;
			if (cnt[kgrid] > 0)
				{
				sdata[ndata++] = (float)(wbnd[0] + sdx * i - bdata_origin_x);
				sdata[ndata++] = (float)(wbnd[2] + sdy * j - bdata_origin_y);
				sdata[ndata++] = (float)gridsmall[kgrid];
				}
			}
	ndata = ndata/3;

	/* do the interpolation */
	cay = (float)tension;
	xmin = (float)(wbnd[0] - 0.5 * sdx - bdata_origin_x);
	ymin = (float)(wbnd[2] - 0.5 * sdy - bdata_origin_y);
	ddx = (float)sdx;
	ddy = (float)sdy;
	fprintf(outfp,"\nDoing Zgrid spline interpolation with %d data points...\n",ndata);
/*for (i=0;i<ndata/3;i++)
{
if (sdata[3*i+2]>2000.0)
fprintf(stderr,"%d %f\n",i,sdata[3*i+2]);
}*/
	mb_zgrid2(sgrid,&sxdim,&sydim,&xmin,&ymin,
		&ddx,&ddy,sdata,&ndata,
		work1,work2,work3,&cay,&sclip);
#endif

	zflag = 5.0e34;
	for (i=0;i<sxdim;i++)
	    for (j=0;j<sydim;j++)
		{
		kgrid = i * sydim + j;
#ifdef USESURFACE
		kint = i + (sydim - j - 1) * sxdim;
#else
		kint = i + j * sxdim;
#endif
		if (cnt[kgrid] == 0)
			{
			gridsmall[kgrid] = sgrid[kint];
/*fprintf(stderr,"YES i:%d j:%d kgrid:%d kint:%d sgrid:%f gridsmall:%f\n",
i,j,kgrid,kint,sgrid[kint],gridsmall[kgrid]);*/
			}
/*		else
			{
fprintf(stderr,"NO  i:%d j:%d kgrid:%d kint:%d sgrid:%f gridsmall:%f\n",
i,j,kgrid,kint,sgrid[kint],gridsmall[kgrid]);
			}*/
		}


	/* deallocate the interpolation arrays */
#ifdef USESURFACE
	mb_freed(verbose,__FILE__,__LINE__,(void **)&sxdata,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&sydata,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&szdata,&error);
#else
	mb_freed(verbose,__FILE__,__LINE__,(void **)&sdata,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&work1,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&work2,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&work3,&error);
#endif
	mb_freed(verbose,__FILE__,__LINE__,(void **)&sgrid,&error);


/*for (i=0;i<sxdim;i++)
	for (j=0;j<sydim;j++)
		{
		kgrid = i * sydim + j;
		kout = i*sydim + j;
		output[kout] = (float) gridsmall[kgrid];
		if (gridsmall[kgrid] >= clipvalue)
			output[kout] = outclipvalue;
		}
zclip = clipvalue;
zmin = zclip;
zmax = zclip;
for (i=0;i<sxdim;i++)
	for (j=0;j<sydim;j++)
		{
		kgrid = i * sydim + j;
		if (zmin == zclip
			&& gridsmall[kgrid] < zclip)
			zmin = gridsmall[kgrid];
		if (zmax == zclip
			&& gridsmall[kgrid] < zclip)
			zmax = gridsmall[kgrid];
		if (gridsmall[kgrid] < zmin && gridsmall[kgrid] < zclip)
			zmin = gridsmall[kgrid];
		if (gridsmall[kgrid] > zmax && gridsmall[kgrid] < zclip)
			zmax = gridsmall[kgrid];
		}
strcpy(ofile,fileroot);
strcat(ofile,"_lorez.grd");
status = write_cdfgrd(verbose,ofile,output,sxdim,sydim,
	wbnd[0],wbnd[1],wbnd[2],wbnd[3],
	zmin,zmax,sdx,sdy,
	xlabel,ylabel,zlabel,title,projection_id,
	argc,argv,&error);*/

	/* do second pass footprint gridding using slope estimates from first pass interpolated grid */

	/* initialize arrays */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			grid[kgrid] = 0.0;
			norm[kgrid] = 0.0;
			sigma[kgrid] = 0.0;
			firsttime[kgrid] = 0.0;
			num[kgrid] = 0;
			cnt[kgrid] = 0;
			}

	/* read in data */
	fprintf(outfp,"\nDoing second pass to generate final grid...\n");
	ndata = 0;
	if ((status = mb_datalist_open(verbose,&datalist,
					filelist,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}
	while ((status = mb_datalist_read2(verbose,datalist,
			&pstatus,path,ppath,&format,&file_weight,&error))
			== MB_SUCCESS)
		{
		ndatafile = 0;

		/* if format > 0 then input is swath sonar file */
		if (format > 0 && path[0] != '#')
		{
		/* apply pstatus */
		if (pstatus == MB_PROCESSED_USE)
			strcpy(file, ppath);
		else
			strcpy(file, path);

		/* check for mbinfo file - get file bounds if possible */
		rformat = format;
		strcpy(rfile,file);
		status = mb_check_info(verbose, rfile, lonflip, bounds,
				&file_in_bounds, &error);
		if (status == MB_FAILURE)
			{
			file_in_bounds = MB_YES;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* initialize the swath sonar file */
		if (file_in_bounds == MB_YES)
		    {
		    /* check for "fast bathymetry" or "fbt" file */
		    if (datatype == MBGRID_DATA_TOPOGRAPHY
			    || datatype == MBGRID_DATA_BATHYMETRY)
			{
			mb_get_fbt(verbose, rfile, &rformat, &error);
			}

		    /* call mb_read_init() */
		    if ((status = mb_read_init(
			verbose,rfile,rformat,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&mbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
			fprintf(outfp,"\nMultibeam File <%s> not initialized for reading\n",rfile);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		    /* get mb_io_ptr */
		    mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
                    
                    /* get topography type */
                    status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);

		    /* allocate memory for reading data arrays */
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(char), (void **)&beamflag, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bath, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&amp, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathlon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathlat, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&ss, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslat, &error);

		    /* if error initializing memory then quit */
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		    /* loop over reading */
		    while (error <= MB_ERROR_NO_ERROR)
			{
			status = mb_read(verbose,mbio_ptr,&kind,
				&rpings,time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&beams_bath,&beams_amp,&pixels_ss,
				beamflag,bath,amp,bathlon,bathlat,
				ss,sslon,sslat,
				comment,&error);

			/* time gaps are not a problem here */
			if (error == MB_ERROR_TIME_GAP)
				{
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
				}

			/* print debug statements */
			if (verbose >= 2)
				{
				fprintf(outfp,"\ndbg2  Ping read in program <%s>\n",program_name);
				fprintf(outfp,"dbg2       kind:           %d\n",kind);
				fprintf(outfp,"dbg2       beams_bath:     %d\n",beams_bath);
				fprintf(outfp,"dbg2       beams_amp:      %d\n",beams_amp);
				fprintf(outfp,"dbg2       pixels_ss:      %d\n",pixels_ss);
				fprintf(outfp,"dbg2       error:          %d\n",error);
				fprintf(outfp,"dbg2       status:         %d\n",status);
				}

			if ((datatype == MBGRID_DATA_BATHYMETRY
				|| datatype == MBGRID_DATA_TOPOGRAPHY)
				&& error == MB_ERROR_NO_ERROR)
			  {

			  /* reproject beam positions if necessary */
			  if (use_projection == MB_YES)
			    {
			    mb_proj_forward(verbose, pjptr,
					    navlon, navlat,
					    &navlon, &navlat,
					    &error);
			    for (ib=0;ib<beams_bath;ib++)
			      if (mb_beam_ok(beamflag[ib]))
				mb_proj_forward(verbose, pjptr,
						bathlon[ib], bathlat[ib],
						&bathlon[ib], &bathlat[ib],
						&error);
			    }

			  /* deal with data */
			  for (ib=0;ib<beams_bath;ib++)
			    if (mb_beam_ok(beamflag[ib]))
			      {
			      /* get position in grid */
			      ix = (bathlon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] + 0.5*dy)/dy;
/*fprintf(outfp, "\nib:%d ix:%d iy:%d   bath: lon:%f lat:%f bath:%f   nav: lon:%f lat:%f\n",
ib, ix, iy, bathlon[ib], bathlat[ib], bath[ib], navlon, navlat);*/

			      /* deal with point data without footprint */
                              if (topo_type != MB_TOPOGRAPHY_TYPE_MULTIBEAM)
                                {
				if (ix >= 0 && ix < gxdim
					  && iy >= 0 && iy < gydim)
				    {
			            kgrid = ix*gydim + iy;
				    norm[kgrid] = norm[kgrid] + file_weight;
				    grid[kgrid] = grid[kgrid]
					    + file_weight*topofactor*bath[ib];
				    sigma[kgrid] = sigma[kgrid]
					    + file_weight*topofactor*topofactor
					    *bath[ib]*bath[ib];
				    num[kgrid]++;
				    cnt[kgrid]++;
				    ndata++;
				    ndatafile++;
				    }
                                }
                                
                              /* else deal with multibeam data that have beam footprints */
                              else
                                {
  
                                /* get slope from low resolution grid */
                                isx = (bathlon[ib] - wbnd[0] + 0.5 * sdx)/sdx;
                                isy = (bathlat[ib] - wbnd[2] + 0.5 * sdy)/sdy;
                                isx = MIN( MAX(isx, 0), sxdim - 1);
                                isy = MIN( MAX(isy, 0), sydim - 1);
                                if (isx == 0)
                                  {
                                  k1 = isx * sydim + isy;
                                  k2 = (isx + 1) * sydim + isy;
                                  dzdx = (gridsmall[k2] - gridsmall[k1]) / sdx;
                                  }
                                else if (isx == sxdim - 1)
                                  {
                                  k1 = (isx - 1) * sydim + isy;
                                  k2 = isx * sydim + isy;
                                  dzdx = (gridsmall[k2] - gridsmall[k1]) / sdx;
                                  }
                                else
                                  {
                                  k1 = (isx - 1) * sydim + isy;
                                  k2 = (isx + 1) * sydim + isy;
                                  dzdx = (gridsmall[k2] - gridsmall[k1]) / (2.0 * sdx);
                                  }
                                if (isy == 0)
                                  {
                                  k1 = isx * sydim + isy;
                                  k2 = isx * sydim + (isy + 1);
                                  dzdy = (gridsmall[k2] - gridsmall[k1]) / sdy;
                                  }
                                else if (isy == sydim - 1)
                                  {
                                  k1 = isx * sydim + (isy - 1);
                                  k2 = isx * sydim + isy;
                                  dzdy = (gridsmall[k2] - gridsmall[k1]) / sdy;
                                  }
                                else
                                  {
                                  k1 = isx * sydim + (isy - 1);
                                  k2 = isx * sydim + (isy + 1);
                                  dzdy = (gridsmall[k2] - gridsmall[k1]) / (2.0 * sdy);
                                  }
  
                                /* check if within allowed time */
                                if (check_time == MB_YES)
                                  {
                                  /* if in region of interest
                                     check if time is ok */
                                  if (ix >= 0 && ix < gxdim
                                    && iy >= 0 && iy < gydim)
                                    {
                                    kgrid = ix*gydim + iy;
                                    if (firsttime[kgrid] <= 0.0)
                                      {
                                      firsttime[kgrid] = time_d;
                                      time_ok = MB_YES;
                                      }
                                    else if (fabs(time_d - firsttime[kgrid])
                                      > timediff)
                                      {
                                      if (first_in_stays == MB_YES)
                                          time_ok = MB_NO;
                                      else
                                          {
                                          time_ok = MB_YES;
                                          firsttime[kgrid] = time_d;
                                          ndata = ndata - cnt[kgrid];
                                          ndatafile = ndatafile - cnt[kgrid];
                                          norm[kgrid] = 0.0;
                                          grid[kgrid] = 0.0;
                                          sigma[kgrid] = 0.0;
                                          num[kgrid] = 0;
                                          cnt[kgrid] = 0;
                                          }
                                      }
                                    else
                                      time_ok = MB_YES;
                                    }
                                  else
                                    time_ok = MB_YES;
                                  }
                                else
                                  time_ok = MB_YES;
  
                                /* process if in region of interest */
                                if (ix >= -xtradim
                                  && ix < gxdim + xtradim
                                  && iy >= -xtradim
                                  && iy < gydim + xtradim
                                  && time_ok == MB_YES)
                                  {
                                  /* calculate footprint - this is a kluge assuming
                                     sonar at surface - also assumes lon lat grid
                                     - to be generalized in later version
                                     DWC 11/16/99 */
                                  /* calculate footprint - now uses sonar altitude
                                     - still assumes lon lat grid
                                     - to be generalized in later version
                                     DWC 1/29/2001 */
                                  /* now handles projected grids
                                     DWC 3/5/2003 */
                                  if (use_projection == MB_YES)
                                    {
                                    foot_dx = (bathlon[ib] - navlon);
                                    foot_dy = (bathlat[ib] - navlat);
                                    }
                                  else
                                    {
                                    foot_dx = (bathlon[ib] - navlon) / mtodeglon;
                                    foot_dy = (bathlat[ib] - navlat) / mtodeglat;
                                    }
                                  foot_lateral = sqrt(foot_dx * foot_dx + foot_dy * foot_dy);
                                  if (foot_lateral > 0.0)
                                      {
                                      foot_dxn = foot_dx / foot_lateral;
                                      foot_dyn = foot_dy / foot_lateral;
                                      }
                                  else
                                      {
                                      foot_dxn = 1.0;
                                      foot_dyn = 0.0;
                                      }
                                  foot_range = sqrt(foot_lateral * foot_lateral + altitude * altitude);
                                  if (foot_range > 0.0)
                                      {
                                      foot_theta = RTD * atan2(foot_lateral, (bath[ib] - sonardepth));
                                      foot_dtheta = 0.5 * scale * mb_io_ptr->beamwidth_xtrack;
                                      foot_dphi = 0.5 * scale * mb_io_ptr->beamwidth_ltrack;
                                      if (foot_dtheta <= 0.0)
                                          foot_dtheta = 1.0;
                                      if (foot_dphi <= 0.0)
                                          foot_dphi = 1.0;
                                      foot_hwidth =(bath[ib] - sonardepth) * tan(DTR * (foot_theta + foot_dtheta))
                                                          - foot_lateral;
                                      foot_hlength = foot_range * tan(DTR * foot_dphi);
  /* fprintf(outfp, "bath:%f sonardepth:%f dx:%f dy:%f lateral:%f range:%f theta:%f dtheta:%f dphi:%f fhwidth:%f fhlength:%f\n",
  bath[ib],sonardepth,foot_dx, foot_dy, foot_lateral, foot_range, foot_theta,foot_dtheta,foot_dphi,foot_hwidth,foot_hlength);*/
  
                                      /* get range of bins around footprint to examine */
                                      if (use_projection == MB_YES)
                                        {
                                        foot_wix = fabs(foot_hwidth * cos(DTR * foot_theta) / dx);
                                        foot_wiy = fabs(foot_hwidth * sin(DTR * foot_theta) / dx);
                                        foot_lix = fabs(foot_hlength * sin(DTR * foot_theta) / dy);
                                        foot_liy = fabs(foot_hlength * cos(DTR * foot_theta) / dy);
                                        }
                                      else
                                        {
                                        foot_wix = fabs(foot_hwidth * cos(DTR * foot_theta) * mtodeglon / dx);
                                        foot_wiy = fabs(foot_hwidth * sin(DTR * foot_theta) * mtodeglon / dx);
                                        foot_lix = fabs(foot_hlength * sin(DTR * foot_theta) * mtodeglat / dy);
                                        foot_liy = fabs(foot_hlength * cos(DTR * foot_theta) * mtodeglat / dy);
                                        }
                                      foot_dix = 2 * MAX(foot_wix, foot_lix);
                                      foot_diy = 2 * MAX(foot_wiy, foot_liy);
  /*fprintf(outfp, "foot_hwidth:%f foot_hlength:%f\n", foot_hwidth, foot_hlength);
  fprintf(outfp, "foot_wix:%d foot_wiy:%d  foot_lix:%d foot_liy:%d    foot_dix:%d foot_diy:%d\n",
  foot_wix, foot_wiy, foot_lix, foot_liy, foot_dix, foot_diy);*/
                                      ix1 = MAX(ix - foot_dix, 0);
                                      ix2 = MIN(ix + foot_dix, gxdim - 1);
                                      iy1 = MAX(iy - foot_diy, 0);
                                      iy2 = MIN(iy + foot_diy, gydim - 1);
  /*fprintf(outfp, "ix1:%d ix2:%d iy1:%d iy2:%d\n", ix1, ix2, iy1, iy2);*/
  
                                      /* loop over neighborhood of bins */
                                      for (ii=ix1;ii<=ix2;ii++)
                                       for (jj=iy1;jj<=iy2;jj++)
                                         {
                                         /* find center of bin in lon lat degrees from sounding center */
                                         kgrid = ii * gydim + jj;
                                         xx = (wbnd[0] + ii*dx + 0.5*dx - bathlon[ib]);
                                         yy = (wbnd[2] + jj*dy + 0.5*dy - bathlat[ib]);
  
                                         /* get depth or topo value at this point using slope estimate */
                                         sbath = topofactor * bath[ib] + dzdx * xx + dzdy * yy;
  /*fprintf(stderr,"ib:%d ii:%d jj:%d bath:%f %f   diff:%f   xx:%f yy:%f dzdx:%f dzdy:%f\n",
  ib,ii,jj,topofactor * bath[ib],sbath,topofactor * bath[ib]-sbath,xx,yy,dzdx,dzdy);*/
  
                                         /* get center and corners of bin in meters from sounding center */
                                        if (use_projection == MB_YES)
                                          {
                                           xx0 = xx;
                                           yy0 = yy;
                                           bdx = 0.5 * dx;
                                           bdy = 0.5 * dy;
                                           }
                                        else
                                          {
                                           xx0 = xx / mtodeglon;
                                           yy0 = yy / mtodeglat;
                                           bdx = 0.5 * dx/ mtodeglon;
                                           bdy = 0.5 * dy/ mtodeglat;
                                           }
                                         xx1 = xx0 - bdx;
                                         xx2 = xx0 + bdx;
                                         yy1 = yy0 - bdy;
                                         yy2 = yy0 + bdy;
  /*fprintf(outfp, "ii:%d jj:%d ix:%d iy:%d xx:%f yy:%f\n", ii, jj, ix, iy, xx, yy);
  fprintf(outfp, "p0: %f %f   p1: %f %f   p2: %f %f\n",
  xx0, yy0, xx1, yy1, xx2, yy2);*/
  
                                         /* rotate center and corners of bin to footprint coordinates */
                                         prx[0] = xx0 * foot_dxn + yy0 * foot_dyn;
                                         pry[0] = -xx0 * foot_dyn + yy0 * foot_dxn;
                                         prx[1] = xx1 * foot_dxn + yy1 * foot_dyn;
                                         pry[1] = -xx1 * foot_dyn + yy1 * foot_dxn;
                                         prx[2] = xx2 * foot_dxn + yy1 * foot_dyn;
                                         pry[2] = -xx2 * foot_dyn + yy1 * foot_dxn;
                                         prx[3] = xx1 * foot_dxn + yy2 * foot_dyn;
                                         pry[3] = -xx1 * foot_dyn + yy2 * foot_dxn;
                                         prx[4] = xx2 * foot_dxn + yy2 * foot_dyn;
                                         pry[4] = -xx2 * foot_dyn + yy2 * foot_dxn;
  
                                         /* get weight integrated over bin */
                                         mbgrid_weight(verbose, foot_hwidth, foot_hlength,
                                                      prx[0], pry[0], bdx, bdy,
                                                      &prx[1], &pry[1],
                                                      &weight, &use_weight, &error);
  
                                         if (use_weight != MBGRID_USE_NO && weight > 0.000001)
                                              {
                                              weight *= file_weight;
                                              norm[kgrid] = norm[kgrid] + weight;
                                              grid[kgrid] = grid[kgrid] + weight * sbath;
                                              sigma[kgrid] = sigma[kgrid] + weight * sbath * sbath;
                                              if (use_weight == MBGRID_USE_YES)
                                                  {
                                                  num[kgrid]++;
                                                  if (ii == ix && jj == iy)
                                                          cnt[kgrid]++;
                                                  }
                                              }
                                         }
                                      ndata++;
                                      ndatafile++;
                                      }
  
                                  /* else for xyz data without footprint */
                                  else if (ix >= 0 && ix < gxdim
                                            && iy >= 0 && iy < gydim)
                                      {
                                      kgrid = ix*gydim + iy;
                                      norm[kgrid] = norm[kgrid] + file_weight;
                                      grid[kgrid] = grid[kgrid]
                                              + file_weight*topofactor*bath[ib];
                                      sigma[kgrid] = sigma[kgrid]
                                              + file_weight*topofactor*topofactor
                                              *bath[ib]*bath[ib];
                                      num[kgrid]++;
                                      cnt[kgrid]++;
                                      ndata++;
                                      ndatafile++;
                                      }
                                  }
                                }
                              }
			  }
			}
		    status = mb_close(verbose,&mbio_ptr,&error);
		    status = MB_SUCCESS;
		    error = MB_ERROR_NO_ERROR;
		    }
		if (verbose >= 2)
			fprintf(outfp,"\n");
		if (verbose > 0 || file_in_bounds == MB_YES)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,rfile);

		/* add to datalist if data actually contributed */
		if (ndatafile > 0 && dfp != NULL)
			{
			if (pstatus == MB_PROCESSED_USE)
				fprintf(dfp, "P:");
			else
				fprintf(dfp, "R:");
			fprintf(dfp, "%s %d %f\n", path, format, file_weight);
			fflush(dfp);
			}
		} /* end if (format > 0) */

		}
	if (datalist != NULL)
		mb_datalist_close(verbose,&datalist,&error);
	if (verbose > 0)
		fprintf(outfp,"\n%d total data points processed\n",ndata);

	/* now loop over all points in the output grid */
	if (verbose >= 1)
		fprintf(outfp,"\nMaking raw grid...\n");
	nbinset = 0;
	nbinzero = 0;
	nbinspline = 0;
	nbinbackground = 0;
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			if (num[kgrid] > 0)
				{
				grid[kgrid] = grid[kgrid]/norm[kgrid];
				factor = sigma[kgrid]/norm[kgrid]
					- grid[kgrid]*grid[kgrid];
				sigma[kgrid] = sqrt(fabs(factor));
				nbinset++;
				}
			else
				{
				grid[kgrid] = clipvalue;
				sigma[kgrid] = 0.0;
				}
			/* fprintf(outfp,"%d %d %d  %f %f %f   %d %d %f %f\n",
			i,j,kgrid,
			grid[kgrid], wbnd[0] + i*dx, wbnd[2] + j*dy,
			num[kgrid],cnt[kgrid],norm[kgrid],sigma[kgrid]);*/
			}

	/***** end of weighted footprint slope gridding *****/
	}

	/***** do weighted footprint gridding *****/
	else if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT)
	{

	/* allocate memory for additional arrays */
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(double),(void **)&norm,&error);

	/* initialize arrays */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			grid[kgrid] = 0.0;
			norm[kgrid] = 0.0;
			sigma[kgrid] = 0.0;
			firsttime[kgrid] = 0.0;
			num[kgrid] = 0;
			cnt[kgrid] = 0;
			}

	/* read in data */
	fprintf(outfp,"\nDoing second pass to generate final grid...\n");
	ndata = 0;
	if ((status = mb_datalist_open(verbose,&datalist,
					filelist,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}
	while ((status = mb_datalist_read2(verbose,datalist,
			&pstatus,path,ppath,&format,&file_weight,&error))
			== MB_SUCCESS)
		{
		ndatafile = 0;

		/* if format > 0 then input is swath sonar file */
		if (format > 0 && path[0] != '#')
		{
		/* apply pstatus */
		if (pstatus == MB_PROCESSED_USE)
			strcpy(file, ppath);
		else
			strcpy(file, path);

		/* check for mbinfo file - get file bounds if possible */
		rformat = format;
		strcpy(rfile,file);
		status = mb_check_info(verbose, rfile, lonflip, bounds,
				&file_in_bounds, &error);
		if (status == MB_FAILURE)
			{
			file_in_bounds = MB_YES;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* initialize the swath sonar file */
		if (file_in_bounds == MB_YES)
		    {
		    /* check for "fast bathymetry" or "fbt" file */
		    if (datatype == MBGRID_DATA_TOPOGRAPHY
			    || datatype == MBGRID_DATA_BATHYMETRY)
			{
			mb_get_fbt(verbose, rfile, &rformat, &error);
			}

		    /* call mb_read_init() */
		    if ((status = mb_read_init(
			verbose,rfile,rformat,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&mbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
			fprintf(outfp,"\nMultibeam File <%s> not initialized for reading\n",rfile);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		    /* get mb_io_ptr */
		    mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
                    
                    /* get topography type */
                    status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);

		    /* allocate memory for reading data arrays */
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(char), (void **)&beamflag, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bath, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&amp, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathlon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathlat, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&ss, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslat, &error);

		    /* if error initializing memory then quit */
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		    /* loop over reading */
		    while (error <= MB_ERROR_NO_ERROR)
			{
			status = mb_read(verbose,mbio_ptr,&kind,
				&rpings,time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&beams_bath,&beams_amp,&pixels_ss,
				beamflag,bath,amp,bathlon,bathlat,
				ss,sslon,sslat,
				comment,&error);

			/* time gaps are not a problem here */
			if (error == MB_ERROR_TIME_GAP)
				{
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
				}

			/* print debug statements */
			if (verbose >= 2)
				{
				fprintf(outfp,"\ndbg2  Ping read in program <%s>\n",program_name);
				fprintf(outfp,"dbg2       kind:           %d\n",kind);
				fprintf(outfp,"dbg2       beams_bath:     %d\n",beams_bath);
				fprintf(outfp,"dbg2       beams_amp:      %d\n",beams_amp);
				fprintf(outfp,"dbg2       pixels_ss:      %d\n",pixels_ss);
				fprintf(outfp,"dbg2       error:          %d\n",error);
				fprintf(outfp,"dbg2       status:         %d\n",status);
				}

			if ((datatype == MBGRID_DATA_BATHYMETRY
				|| datatype == MBGRID_DATA_TOPOGRAPHY)
				&& error == MB_ERROR_NO_ERROR)
			  {

			  /* reproject beam positions if necessary */
			  if (use_projection == MB_YES)
			    {
			    mb_proj_forward(verbose, pjptr,
					    navlon, navlat,
					    &navlon, &navlat,
					    &error);
			    for (ib=0;ib<beams_bath;ib++)
			      if (mb_beam_ok(beamflag[ib]))
				mb_proj_forward(verbose, pjptr,
						bathlon[ib], bathlat[ib],
						&bathlon[ib], &bathlat[ib],
						&error);
			    }

			  /* deal with data */
			  for (ib=0;ib<beams_bath;ib++)
			    if (mb_beam_ok(beamflag[ib]))
			      {
			      /* get position in grid */
			      ix = (bathlon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] + 0.5*dy)/dy;
/*fprintf(outfp, "\nib:%d ix:%d iy:%d   bath: lon:%f lat:%f bath:%f   nav: lon:%f lat:%f\n",
ib, ix, iy, bathlon[ib], bathlat[ib], bath[ib], navlon, navlat);*/

			      /* check if within allowed time */
			      if (check_time == MB_YES)
			        {
				/* if in region of interest
				   check if time is ok */
				if (ix >= 0 && ix < gxdim
				  && iy >= 0 && iy < gydim)
				  {
			          kgrid = ix*gydim + iy;
				  if (firsttime[kgrid] <= 0.0)
				    {
				    firsttime[kgrid] = time_d;
				    time_ok = MB_YES;
				    }
				  else if (fabs(time_d - firsttime[kgrid])
				    > timediff)
				    {
				    if (first_in_stays == MB_YES)
					time_ok = MB_NO;
				    else
					{
					time_ok = MB_YES;
					firsttime[kgrid] = time_d;
					ndata = ndata - cnt[kgrid];
					ndatafile = ndatafile - cnt[kgrid];
					norm[kgrid] = 0.0;
					grid[kgrid] = 0.0;
					sigma[kgrid] = 0.0;
					num[kgrid] = 0;
					cnt[kgrid] = 0;
					}
				    }
			          else
				    time_ok = MB_YES;
				  }
				else
				  time_ok = MB_YES;
				}
			      else
				time_ok = MB_YES;

			      /* process if in region of interest */
			      if (ix >= -xtradim
				&& ix < gxdim + xtradim
				&& iy >= -xtradim
				&& iy < gydim + xtradim
				&& time_ok == MB_YES)
			        {
                                /* deal with point data without footprint */
                                if (topo_type != MB_TOPOGRAPHY_TYPE_MULTIBEAM)
                                    {
                                    kgrid = ix*gydim + iy;
                                    norm[kgrid] = norm[kgrid] + file_weight;
                                    grid[kgrid] = grid[kgrid]
                                            + file_weight*topofactor*bath[ib];
                                    sigma[kgrid] = sigma[kgrid]
                                            + file_weight*topofactor*topofactor
                                            *bath[ib]*bath[ib];
                                    num[kgrid]++;
                                    cnt[kgrid]++;
                                    ndata++;
                                    ndatafile++;
                                    }
                                  
                                /* else deal with multibeam data that have beam footprints */
                                else
                                    {
                                    /* calculate footprint - this is a kluge assuming
                                       sonar at surface - also assumes lon lat grid
                                       - to be generalized in later version
                                       DWC 11/16/99 */
                                    /* calculate footprint - now uses sonar altitude
                                       - still assumes lon lat grid
                                       - to be generalized in later version
                                       DWC 1/29/2001 */
                                    /* now handles projected grids
                                       DWC 3/5/2003 */
                                    if (use_projection == MB_YES)
                                      {
                                      foot_dx = (bathlon[ib] - navlon);
                                      foot_dy = (bathlat[ib] - navlat);
                                      }
                                    else
                                      {
                                      foot_dx = (bathlon[ib] - navlon) / mtodeglon;
                                      foot_dy = (bathlat[ib] - navlat) / mtodeglat;
                                      }
                                    foot_lateral = sqrt(foot_dx * foot_dx + foot_dy * foot_dy);
                                    if (foot_lateral > 0.0)
                                        {
                                        foot_dxn = foot_dx / foot_lateral;
                                        foot_dyn = foot_dy / foot_lateral;
                                        }
                                    else
                                        {
                                        foot_dxn = 1.0;
                                        foot_dyn = 0.0;
                                        }
                                    foot_range = sqrt(foot_lateral * foot_lateral + altitude * altitude);
                                    if (foot_range > 0.0)
                                        {
                                        foot_theta = RTD * atan2(foot_lateral, (bath[ib] - sonardepth));
                                        foot_dtheta = 0.5 * scale * mb_io_ptr->beamwidth_xtrack;
                                        foot_dphi = 0.5 * scale * mb_io_ptr->beamwidth_ltrack;
                                        if (foot_dtheta <= 0.0)
                                            foot_dtheta = 1.0;
                                        if (foot_dphi <= 0.0)
                                            foot_dphi = 1.0;
                                        foot_hwidth =(bath[ib] - sonardepth) * tan(DTR * (foot_theta + foot_dtheta))
                                                            - foot_lateral;
                                        foot_hlength = foot_range * tan(DTR * foot_dphi);
    /* fprintf(outfp, "bath:%f sonardepth:%f dx:%f dy:%f lateral:%f range:%f theta:%f dtheta:%f dphi:%f fhwidth:%f fhlength:%f\n",
    bath[ib],sonardepth,foot_dx, foot_dy, foot_lateral, foot_range, foot_theta,foot_dtheta,foot_dphi,foot_hwidth,foot_hlength);*/
    
                                        /* get range of bins around footprint to examine */
                                        if (use_projection == MB_YES)
                                          {
                                          foot_wix = fabs(foot_hwidth * cos(DTR * foot_theta) / dx);
                                          foot_wiy = fabs(foot_hwidth * sin(DTR * foot_theta) / dx);
                                          foot_lix = fabs(foot_hlength * sin(DTR * foot_theta) / dy);
                                          foot_liy = fabs(foot_hlength * cos(DTR * foot_theta) / dy);
                                          }
                                        else
                                          {
                                          foot_wix = fabs(foot_hwidth * cos(DTR * foot_theta) * mtodeglon / dx);
                                          foot_wiy = fabs(foot_hwidth * sin(DTR * foot_theta) * mtodeglon / dx);
                                          foot_lix = fabs(foot_hlength * sin(DTR * foot_theta) * mtodeglat / dy);
                                          foot_liy = fabs(foot_hlength * cos(DTR * foot_theta) * mtodeglat / dy);
                                          }
                                        foot_dix = 2 * MAX(foot_wix, foot_lix);
                                        foot_diy = 2 * MAX(foot_wiy, foot_liy);
    /*fprintf(outfp, "foot_hwidth:%f foot_hlength:%f\n", foot_hwidth, foot_hlength);
    fprintf(outfp, "foot_wix:%d foot_wiy:%d  foot_lix:%d foot_liy:%d    foot_dix:%d foot_diy:%d\n",
    foot_wix, foot_wiy, foot_lix, foot_liy, foot_dix, foot_diy);*/
                                        ix1 = MAX(ix - foot_dix, 0);
                                        ix2 = MIN(ix + foot_dix, gxdim - 1);
                                        iy1 = MAX(iy - foot_diy, 0);
                                        iy2 = MIN(iy + foot_diy, gydim - 1);
    /*fprintf(outfp, "ix1:%d ix2:%d iy1:%d iy2:%d\n", ix1, ix2, iy1, iy2);*/
    
                                        /* loop over neighborhood of bins */
                                        for (ii=ix1;ii<=ix2;ii++)
                                         for (jj=iy1;jj<=iy2;jj++)
                                           {
                                           /* find center of bin in lon lat degrees from sounding center */
                                           kgrid = ii * gydim + jj;
                                           xx = (wbnd[0] + ii*dx + 0.5*dx - bathlon[ib]);
                                           yy = (wbnd[2] + jj*dy + 0.5*dy - bathlat[ib]);
    
                                           /* get depth or topo value at this point */
                                           sbath = topofactor * bath[ib];
    /*fprintf(stderr,"ib:%d ii:%d jj:%d bath:%f %f   diff:%f   xx:%f yy:%f\n",
    ib,ii,jj,topofactor * bath[ib],sbath,topofactor * bath[ib]-sbath,xx,yy);*/
    
                                           /* get center and corners of bin in meters from sounding center */
                                          if (use_projection == MB_YES)
                                            {
                                             xx0 = xx;
                                             yy0 = yy;
                                             bdx = 0.5 * dx;
                                             bdy = 0.5 * dy;
                                             }
                                          else
                                            {
                                             xx0 = xx / mtodeglon;
                                             yy0 = yy / mtodeglat;
                                             bdx = 0.5 * dx/ mtodeglon;
                                             bdy = 0.5 * dy/ mtodeglat;
                                             }
                                           xx1 = xx0 - bdx;
                                           xx2 = xx0 + bdx;
                                           yy1 = yy0 - bdy;
                                           yy2 = yy0 + bdy;
    /*fprintf(outfp, "ii:%d jj:%d ix:%d iy:%d xx:%f yy:%f\n", ii, jj, ix, iy, xx, yy);
    fprintf(outfp, "p0: %f %f   p1: %f %f   p2: %f %f\n",
    xx0, yy0, xx1, yy1, xx2, yy2);*/
    
                                           /* rotate center and corners of bin to footprint coordinates */
                                           prx[0] = xx0 * foot_dxn + yy0 * foot_dyn;
                                           pry[0] = -xx0 * foot_dyn + yy0 * foot_dxn;
                                           prx[1] = xx1 * foot_dxn + yy1 * foot_dyn;
                                           pry[1] = -xx1 * foot_dyn + yy1 * foot_dxn;
                                           prx[2] = xx2 * foot_dxn + yy1 * foot_dyn;
                                           pry[2] = -xx2 * foot_dyn + yy1 * foot_dxn;
                                           prx[3] = xx1 * foot_dxn + yy2 * foot_dyn;
                                           pry[3] = -xx1 * foot_dyn + yy2 * foot_dxn;
                                           prx[4] = xx2 * foot_dxn + yy2 * foot_dyn;
                                           pry[4] = -xx2 * foot_dyn + yy2 * foot_dxn;
    
                                           /* get weight integrated over bin */
                                           mbgrid_weight(verbose, foot_hwidth, foot_hlength,
                                                        prx[0], pry[0], bdx, bdy,
                                                        &prx[1], &pry[1],
                                                        &weight, &use_weight, &error);
    
                                           if (use_weight != MBGRID_USE_NO && weight > 0.000001)
                                                {
                                                weight *= file_weight;
                                                norm[kgrid] = norm[kgrid] + weight;
                                                grid[kgrid] = grid[kgrid] + weight * sbath;
                                                sigma[kgrid] = sigma[kgrid] + weight * sbath * sbath;
                                                if (use_weight == MBGRID_USE_YES)
                                                    {
                                                    num[kgrid]++;
                                                    if (ii == ix && jj == iy)
                                                            cnt[kgrid]++;
                                                    }
                                                }
                                           }
                                        ndata++;
                                        ndatafile++;
                                        }
    
                                    /* else for xyz data without footprint */
                                    else if (ix >= 0 && ix < gxdim
                                              && iy >= 0 && iy < gydim)
                                        {
                                        kgrid = ix*gydim + iy;
                                        norm[kgrid] = norm[kgrid] + file_weight;
                                        grid[kgrid] = grid[kgrid]
                                                + file_weight*topofactor*bath[ib];
                                        sigma[kgrid] = sigma[kgrid]
                                                + file_weight*topofactor*topofactor
                                                *bath[ib]*bath[ib];
                                        num[kgrid]++;
                                        cnt[kgrid]++;
                                        ndata++;
                                        ndatafile++;
                                        }
                                    }
				}
			      }
			  }
			}
		    status = mb_close(verbose,&mbio_ptr,&error);
		    status = MB_SUCCESS;
		    error = MB_ERROR_NO_ERROR;
		    }
		if (verbose >= 2)
			fprintf(outfp,"\n");
		if (verbose > 0 || file_in_bounds == MB_YES)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,rfile);

		/* add to datalist if data actually contributed */
		if (ndatafile > 0 && dfp != NULL)
			{
			if (pstatus == MB_PROCESSED_USE)
				fprintf(dfp, "P:");
			else
				fprintf(dfp, "R:");
			fprintf(dfp, "%s %d %f\n", path, format, file_weight);
			fflush(dfp);
			}
		} /* end if (format > 0) */

		}
	if (datalist != NULL)
		mb_datalist_close(verbose,&datalist,&error);
	if (verbose > 0)
		fprintf(outfp,"\n%d total data points processed\n",ndata);

	/* now loop over all points in the output grid */
	if (verbose >= 1)
		fprintf(outfp,"\nMaking raw grid...\n");
	nbinset = 0;
	nbinzero = 0;
	nbinspline = 0;
	nbinbackground = 0;
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			if (num[kgrid] > 0)
				{
				grid[kgrid] = grid[kgrid]/norm[kgrid];
				factor = sigma[kgrid]/norm[kgrid]
					- grid[kgrid]*grid[kgrid];
				sigma[kgrid] = sqrt(fabs(factor));
				nbinset++;
				}
			else
				{
				grid[kgrid] = clipvalue;
				sigma[kgrid] = 0.0;
				}
			/* fprintf(outfp,"%d %d %d  %f %f %f   %d %d %f %f\n",
			i,j,kgrid,
			grid[kgrid], wbnd[0] + i*dx, wbnd[2] + j*dy,
			num[kgrid],cnt[kgrid],norm[kgrid],sigma[kgrid]);*/
			}

	/***** end of weighted footprint gridding *****/
	}

	/***** do weighted mean or min/max gridding *****/
	else if (grid_mode != MBGRID_MEDIAN_FILTER)
	{

	/* allocate memory for additional arrays */
	if (status == MB_SUCCESS)
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(double),(void **)&norm,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}

	/* initialize arrays */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			grid[kgrid] = 0.0;
			norm[kgrid] = 0.0;
			sigma[kgrid] = 0.0;
			firsttime[kgrid] = 0.0;
			num[kgrid] = 0;
			cnt[kgrid] = 0;
			}

	/* read in data */
	ndata = 0;
	if ((status = mb_datalist_open(verbose,&datalist,
					filelist,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}
	while ((status = mb_datalist_read2(verbose,datalist,
			&pstatus,path,ppath,&format,&file_weight,&error))
			== MB_SUCCESS)
		{
		ndatafile = 0;

		/* if format > 0 then input is swath sonar file */
		if (format > 0 && path[0] != '#')
		{
		/* apply pstatus */
		if (pstatus == MB_PROCESSED_USE)
			strcpy(file, ppath);
		else
			strcpy(file, path);

		/* check for mbinfo file - get file bounds if possible */
		rformat = format;
		strcpy(rfile,file);
		status = mb_check_info(verbose, rfile, lonflip, bounds,
				&file_in_bounds, &error);
		if (status == MB_FAILURE)
			{
			file_in_bounds = MB_YES;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* initialize the swath sonar file */
		if (file_in_bounds == MB_YES)
		    {
		    /* check for "fast bathymetry" or "fbt" file */
		    if (datatype == MBGRID_DATA_TOPOGRAPHY
			    || datatype == MBGRID_DATA_BATHYMETRY)
			{
			mb_get_fbt(verbose, rfile, &rformat, &error);
			}

		    /* call mb_read_init() */
		    if ((status = mb_read_init(
			verbose,rfile,rformat,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&mbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
			fprintf(outfp,"\nMultibeam File <%s> not initialized for reading\n",rfile);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		    /* allocate memory for reading data arrays */
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(char), (void **)&beamflag, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bath, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&amp, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathlon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathlat, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&ss, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslat, &error);

		    /* if error initializing memory then quit */
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		    /* loop over reading */
		    while (error <= MB_ERROR_NO_ERROR)
			{
			status = mb_read(verbose,mbio_ptr,&kind,
				&rpings,time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&beams_bath,&beams_amp,&pixels_ss,
				beamflag,bath,amp,bathlon,bathlat,
				ss,sslon,sslat,
				comment,&error);

			/* time gaps are not a problem here */
			if (error == MB_ERROR_TIME_GAP)
				{
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
				}

			/* print debug statements */
			if (verbose >= 2)
				{
				fprintf(outfp,"\ndbg2  Ping read in program <%s>\n",program_name);
				fprintf(outfp,"dbg2       kind:           %d\n",kind);
				fprintf(outfp,"dbg2       beams_bath:     %d\n",beams_bath);
				fprintf(outfp,"dbg2       beams_amp:      %d\n",beams_amp);
				fprintf(outfp,"dbg2       pixels_ss:      %d\n",pixels_ss);
				fprintf(outfp,"dbg2       error:          %d\n",error);
				fprintf(outfp,"dbg2       status:         %d\n",status);
				}

			if ((datatype == MBGRID_DATA_BATHYMETRY
				|| datatype == MBGRID_DATA_TOPOGRAPHY)
				&& error == MB_ERROR_NO_ERROR)
			  {

			  /* reproject beam positions if necessary */
			  if (use_projection == MB_YES)
			    {
			    for (ib=0;ib<beams_bath;ib++)
			      if (mb_beam_ok(beamflag[ib]))
				mb_proj_forward(verbose, pjptr,
						bathlon[ib], bathlat[ib],
						&bathlon[ib], &bathlat[ib],
						&error);
			    }

			  /* deal with data */
			  for (ib=0;ib<beams_bath;ib++)
			    if (mb_beam_ok(beamflag[ib]))
			      {
			      /* get position in grid */
			      ix = (bathlon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] + 0.5*dy)/dy;
/* if (ib==beams_bath/2)fprintf(outfp, "ib:%d ix:%d iy:%d   bath: lon:%.10f lat:%.10f bath:%f   dx:%.10f dy:%.10f  origin: lon:%.10f lat:%.10f\n",
ib, ix, iy, bathlon[ib], bathlat[ib], bath[ib], dx, dy, wbnd[0], wbnd[1]); */

			      /* check if within allowed time */
			      if (check_time == MB_YES)
			        {
				/* if in region of interest
				   check if time is ok */
				if (ix >= 0 && ix < gxdim
				  && iy >= 0 && iy < gydim)
				  {
			          kgrid = ix*gydim + iy;
				  if (firsttime[kgrid] <= 0.0)
				    {
				    firsttime[kgrid] = time_d;
				    time_ok = MB_YES;
				    }
				  else if (fabs(time_d - firsttime[kgrid])
				    > timediff)
				    {
				    if (first_in_stays == MB_YES)
					time_ok = MB_NO;
				    else
					{
					time_ok = MB_YES;
					firsttime[kgrid] = time_d;
					ndata = ndata - cnt[kgrid];
					ndatafile = ndatafile - cnt[kgrid];
					norm[kgrid] = 0.0;
					grid[kgrid] = 0.0;
					sigma[kgrid] = 0.0;
					num[kgrid] = 0;
					cnt[kgrid] = 0;
					}
				    }
			          else
				    time_ok = MB_YES;
				  }
				else
				  time_ok = MB_YES;
				}
			      else
				time_ok = MB_YES;

			      /* process if in region of interest */
			      if (grid_mode == MBGRID_WEIGHTED_MEAN
			        && ix >= -xtradim
				&& ix < gxdim + xtradim
				&& iy >= -xtradim
				&& iy < gydim + xtradim
				&& time_ok == MB_YES)
			        {
			        ix1 = MAX(ix - xtradim, 0);
			        ix2 = MIN(ix + xtradim, gxdim - 1);
			        iy1 = MAX(iy - xtradim, 0);
			        iy2 = MIN(iy + xtradim, gydim - 1);
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				   {
				   kgrid = ii*gydim + jj;
				   xx = wbnd[0] + ii*dx - bathlon[ib];
				   yy = wbnd[2] + jj*dy - bathlat[ib];
				   weight = file_weight * exp(-(xx*xx + yy*yy)*factor);
				   norm[kgrid] = norm[kgrid] + weight;
				   grid[kgrid] = grid[kgrid]
					+ weight*topofactor*bath[ib];
				   sigma[kgrid] = sigma[kgrid]
					+ weight*topofactor*topofactor
					*bath[ib]*bath[ib];
				   num[kgrid]++;
				   if (ii == ix && jj == iy)
					cnt[kgrid]++;
				   }
				ndata++;
				ndatafile++;
				}
			      else if (ix >= 0
				&& ix < gxdim
				&& iy >= 0
				&& iy < gydim
				&& time_ok == MB_YES)
			        {
				kgrid = ix*gydim + iy;
				if ((num[kgrid] > 0
				  && grid_mode == MBGRID_MINIMUM_FILTER
				  && grid[kgrid] > topofactor*bath[ib])
				  || (num[kgrid] > 0
				  && grid_mode == MBGRID_MAXIMUM_FILTER
				  && grid[kgrid] < topofactor*bath[ib])
				  || num[kgrid] <= 0)
				  {
				  norm[kgrid] = 1.0;
				  grid[kgrid] = topofactor*bath[ib];
				  sigma[kgrid] = topofactor*topofactor
					    *bath[ib]*bath[ib];
				  num[kgrid] = 1;
				  cnt[kgrid] = 1;
				  }
				ndata++;
				ndatafile++;
				}
			      }
			  }
			else if (datatype == MBGRID_DATA_AMPLITUDE
				&& error == MB_ERROR_NO_ERROR)
			  {

			  /* reproject beam positions if necessary */
			  if (use_projection == MB_YES)
			    {
			    for (ib=0;ib<beams_amp;ib++)
			      if (mb_beam_ok(beamflag[ib]))
				mb_proj_forward(verbose, pjptr,
						bathlon[ib], bathlat[ib],
						&bathlon[ib], &bathlat[ib],
						&error);
			    }

			  /* deal with data */
			  for (ib=0;ib<beams_amp;ib++)
			    if (mb_beam_ok(beamflag[ib]))
			      {
			      /* get position in grid */
			      ix = (bathlon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] + 0.5*dy)/dy;

			      /* check if within allowed time */
			      if (check_time == MB_YES)
			        {
				/* if in region of interest
				   check if time is ok */
				if (ix >= 0 && ix < gxdim
				  && iy >= 0 && iy < gydim)
				  {
			          kgrid = ix*gydim + iy;
				  if (firsttime[kgrid] <= 0.0)
				    {
				    firsttime[kgrid] = time_d;
				    time_ok = MB_YES;
				    }
				  else if (fabs(time_d - firsttime[kgrid])
				    > timediff)
				    {
				    if (first_in_stays == MB_YES)
					time_ok = MB_NO;
				    else
					{
					time_ok = MB_YES;
					firsttime[kgrid] = time_d;
					ndata = ndata - cnt[kgrid];
					ndatafile = ndatafile - cnt[kgrid];
					norm[kgrid] = 0.0;
					grid[kgrid] = 0.0;
					sigma[kgrid] = 0.0;
					num[kgrid] = 0;
					cnt[kgrid] = 0;
					}
				    }
			          else
				    time_ok = MB_YES;
				  }
				else
				  time_ok = MB_YES;
				}
			      else
				time_ok = MB_YES;

			      /* process if in region of interest */
			      if (grid_mode == MBGRID_WEIGHTED_MEAN
			        && ix >= -xtradim
				&& ix < gxdim + xtradim
				&& iy >= -xtradim
				&& iy < gydim + xtradim
				&& time_ok == MB_YES)
			        {
			        ix1 = MAX(ix - xtradim, 0);
			        ix2 = MIN(ix + xtradim, gxdim - 1);
			        iy1 = MAX(iy - xtradim, 0);
			        iy2 = MIN(iy + xtradim, gydim - 1);
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				   {
				   kgrid = ii*gydim + jj;
				   xx = wbnd[0] + ii*dx - bathlon[ib];
				   yy = wbnd[2] + jj*dy - bathlat[ib];
				   weight = file_weight * exp(-(xx*xx + yy*yy)*factor);
				   norm[kgrid] = norm[kgrid] + weight;
				   grid[kgrid] = grid[kgrid] + weight*amp[ib];
				   sigma[kgrid] = sigma[kgrid]
					+ weight*amp[ib]*amp[ib];
				   num[kgrid]++;
				   if (ii == ix && jj == iy)
					cnt[kgrid]++;
				   }
				ndata++;
				ndatafile++;
				}
			      else if (ix >= 0
				&& ix < gxdim
				&& iy >= 0
				&& iy < gydim
				&& time_ok == MB_YES)
			        {
				kgrid = ix*gydim + iy;
				if ((num[kgrid] > 0
				  && grid_mode == MBGRID_MINIMUM_FILTER
				  && grid[kgrid] > amp[ib])
				  || (num[kgrid] > 0
				  && grid_mode == MBGRID_MAXIMUM_FILTER
				  && grid[kgrid] < amp[ib])
				  || num[kgrid] <= 0)
				  {
				  norm[kgrid] = 1.0;
				  grid[kgrid] = amp[ib];
				  sigma[kgrid] = amp[ib]*amp[ib];
				  num[kgrid] = 1;
				  cnt[kgrid] = 1;
				  }
				ndata++;
				ndatafile++;
				}
			      }
			  }
			else if (datatype == MBGRID_DATA_SIDESCAN
				&& error == MB_ERROR_NO_ERROR)
			  {

			  /* reproject pixel positions if necessary */
			  if (use_projection == MB_YES)
			    {
			    for (ib=0;ib<pixels_ss;ib++)
			      if (ss[ib] > MB_SIDESCAN_NULL)
				mb_proj_forward(verbose, pjptr,
						sslon[ib], sslat[ib],
						&sslon[ib], &sslat[ib],
						&error);
			    }

			  /* deal with data */
			  for (ib=0;ib<pixels_ss;ib++)
			    if (ss[ib] > MB_SIDESCAN_NULL)
			      {
			      /* get position in grid */
			      ix = (sslon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (sslat[ib] - wbnd[2] + 0.5*dy)/dy;

			      /* check if within allowed time */
			      if (check_time == MB_YES)
			        {
				/* if in region of interest
				   check if time is ok */
				if (ix >= 0 && ix < gxdim
				  && iy >= 0 && iy < gydim)
				  {
			          kgrid = ix*gydim + iy;
				  if (firsttime[kgrid] <= 0.0)
				    {
				    firsttime[kgrid] = time_d;
				    time_ok = MB_YES;
				    }
				  else if (fabs(time_d - firsttime[kgrid])
				    > timediff)
				    {
				    if (first_in_stays == MB_YES)
					time_ok = MB_NO;
				    else
					{
					time_ok = MB_YES;
					firsttime[kgrid] = time_d;
					ndata = ndata - cnt[kgrid];
					ndatafile = ndatafile - cnt[kgrid];
					norm[kgrid] = 0.0;
					grid[kgrid] = 0.0;
					sigma[kgrid] = 0.0;
					num[kgrid] = 0;
					cnt[kgrid] = 0;
					}
				    }
			          else
				    time_ok = MB_YES;
				  }
				else
				  time_ok = MB_YES;
				}
			      else
				time_ok = MB_YES;

			      /* process if in region of interest */
			      if (grid_mode == MBGRID_WEIGHTED_MEAN
			        && ix >= -xtradim
				&& ix < gxdim + xtradim
				&& iy >= -xtradim
				&& iy < gydim + xtradim
				&& time_ok == MB_YES)
			        {
			        ix1 = MAX(ix - xtradim, 0);
			        ix2 = MIN(ix + xtradim, gxdim - 1);
			        iy1 = MAX(iy - xtradim, 0);
			        iy2 = MIN(iy + xtradim, gydim - 1);
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				   {
				   kgrid = ii*gydim + jj;
				   xx = wbnd[0] + ii*dx - sslon[ib];
				   yy = wbnd[2] + jj*dy - sslat[ib];
				   weight = file_weight * exp(-(xx*xx + yy*yy)*factor);
				   norm[kgrid] = norm[kgrid] + weight;
				   grid[kgrid] = grid[kgrid] + weight*ss[ib];
				   sigma[kgrid] = sigma[kgrid]
					+ weight*ss[ib]*ss[ib];
				   num[kgrid]++;
				   if (ii == ix && jj == iy)
					cnt[kgrid]++;
				   }
				ndata++;
				ndatafile++;
				}
			      else if (ix >= 0
				&& ix < gxdim
				&& iy >= 0
				&& iy < gydim
				&& time_ok == MB_YES)
			        {
				kgrid = ix*gydim + iy;
				if ((num[kgrid] > 0
				  && grid_mode == MBGRID_MINIMUM_FILTER
				  && grid[kgrid] > ss[ib])
				  || (num[kgrid] > 0
				  && grid_mode == MBGRID_MAXIMUM_FILTER
				  && grid[kgrid] < ss[ib])
				  || num[kgrid] <= 0)
				  {
				  norm[kgrid] = 1.0;
				  grid[kgrid] = ss[ib];
				  sigma[kgrid] = ss[ib]*ss[ib];
				  num[kgrid] = 1;
				  cnt[kgrid] = 1;
				  }
				ndata++;
				ndatafile++;
				}
			      }
			  }
			}
		    status = mb_close(verbose,&mbio_ptr,&error);
		    status = MB_SUCCESS;
		    error = MB_ERROR_NO_ERROR;
		    }
		if (verbose >= 2)
			fprintf(outfp,"\n");
		if (verbose > 0 || file_in_bounds == MB_YES)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,rfile);

		/* add to datalist if data actually contributed */
		if (ndatafile > 0 && dfp != NULL)
			{
			if (pstatus == MB_PROCESSED_USE)
				fprintf(dfp, "P:");
			else
				fprintf(dfp, "R:");
			fprintf(dfp, "%s %d %f\n", path, format, file_weight);
			fflush(dfp);
			}
		} /* end if (format > 0) */

		/* if format == 0 then input is lon,lat,values triples file */
		else if (format == 0 && path[0] != '#')
		{
		/* open data file */
		if ((rfp = fopen(path,"r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(outfp,"\nUnable to open lon,lat,value triples data file1: %s\n",
				path);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		/* loop over reading */
		while (fscanf(rfp,"%lf %lf %lf",&tlon,&tlat,&tvalue) != EOF)
			{
			  /* reproject data positions if necessary */
			  if (use_projection == MB_YES)
				mb_proj_forward(verbose, pjptr,
						tlon, tlat,
						&tlon, &tlat,
						&error);

			  /* get position in grid */
			  ix = (tlon - wbnd[0] + 0.5*dx)/dx;
			  iy = (tlat - wbnd[2] + 0.5*dy)/dy;

			  /* check if overwriting */
			  if (check_time == MB_YES)
			    {
			    /* if in region of interest
			       check if overwriting */
			    if (ix >= 0 && ix < gxdim
			      && iy >= 0 && iy < gydim)
			      {
			      kgrid = ix*gydim + iy;
			      if (firsttime[kgrid] > 0.0)
				time_ok = MB_NO;
			      else
				time_ok = MB_YES;
			      }
			    else
			      time_ok = MB_YES;
			    }
			  else
			    time_ok = MB_YES;

			  /* process the data */
			  if (grid_mode == MBGRID_WEIGHTED_MEAN
			    && ix >= -xtradim
			    && ix < gxdim + xtradim
			    && iy >= -xtradim
			    && iy < gydim + xtradim
			    && time_ok == MB_YES)
			    {
			    ix1 = MAX(ix - xtradim, 0);
			    ix2 = MIN(ix + xtradim, gxdim - 1);
			    iy1 = MAX(iy - xtradim, 0);
			    iy2 = MIN(iy + xtradim, gydim - 1);
			    for (ii=ix1;ii<=ix2;ii++)
			     for (jj=iy1;jj<=iy2;jj++)
			       {
			       kgrid = ii*gydim + jj;
			       xx = wbnd[0] + ii*dx - tlon;
			       yy = wbnd[2] + jj*dy - tlat;
			       weight = file_weight * exp(-(xx*xx + yy*yy)*factor);
			       norm[kgrid] = norm[kgrid] + weight;
			       grid[kgrid] = grid[kgrid]
				    + weight*topofactor*tvalue;
			       sigma[kgrid] = sigma[kgrid]
				    + weight*topofactor*topofactor
				    *tvalue*tvalue;
			       num[kgrid]++;
			       if (ii == ix && jj == iy)
				    cnt[kgrid]++;
			       }
			    ndata++;
			    ndatafile++;
			    }
			  else if (ix >= 0
			    && ix < gxdim
			    && iy >= 0
			    && iy < gydim
			    && time_ok == MB_YES)
			    {
			    kgrid = ix*gydim + iy;
			    if ((num[kgrid] > 0
			      && grid_mode == MBGRID_MINIMUM_FILTER
			      && grid[kgrid] > topofactor*tvalue)
			      || (num[kgrid] > 0
			      && grid_mode == MBGRID_MAXIMUM_FILTER
			      && grid[kgrid] < topofactor*tvalue)
			      || num[kgrid] <= 0)
			      {
			      norm[kgrid] = 1.0;
			      grid[kgrid] = topofactor*tvalue;
			      sigma[kgrid] = topofactor*topofactor
					*tvalue*tvalue;
			      num[kgrid] = 1;
			      cnt[kgrid] = 1;
			      }
			    ndata++;
			    ndatafile++;
			    }
			}
		fclose(rfp);
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		if (verbose >= 2)
			fprintf(outfp,"\n");
		if (verbose > 0)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,file);

		/* add to datalist if data actually contributed */
		if (ndatafile > 0 && dfp != NULL)
			{
			if (pstatus == MB_PROCESSED_USE)
				fprintf(dfp, "P:");
			else
				fprintf(dfp, "R:");
			fprintf(dfp, "%s %d %f\n", path, format, file_weight);
			fflush(dfp);
			}
		} /* end if (format == 0) */

		}
	if (datalist != NULL)
		mb_datalist_close(verbose,&datalist,&error);
	if (verbose > 0)
		fprintf(outfp,"\n%d total data points processed\n",ndata);

	/* now loop over all points in the output grid */
	if (verbose >= 1)
		fprintf(outfp,"\nMaking raw grid...\n");
	nbinset = 0;
	nbinzero = 0;
	nbinspline = 0;
	nbinbackground = 0;
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			if (cnt[kgrid] > 0)
				{
				grid[kgrid] = grid[kgrid]/norm[kgrid];
				factor = sigma[kgrid]/norm[kgrid]
					- grid[kgrid]*grid[kgrid];
				sigma[kgrid] = sqrt(fabs(factor));
				nbinset++;
				}
			else
				{
				grid[kgrid] = clipvalue;
				sigma[kgrid] = 0.0;
				}
			/*fprintf(outfp,"%d %d %d  %f %f %f   %d %d %f %f\n",
				i,j,kgrid,
				grid[kgrid], wbnd[0] + i*dx, wbnd[2] + j*dy,
				num[kgrid],cnt[kgrid],norm[kgrid],sigma[kgrid]);*/
			}

	/***** end of weighted mean gridding *****/
	}

	/***** else do median filtering gridding *****/
	else if (grid_mode == MBGRID_MEDIAN_FILTER)
	{

	/* allocate memory for additional arrays */
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(double *),(void **)&data,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}

	/* initialize arrays */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			grid[kgrid] = 0.0;
			sigma[kgrid] = 0.0;
			firsttime[kgrid] = 0.0;
			cnt[kgrid] = 0;
			num[kgrid] = 0;
			data[kgrid] = NULL;
			}

	/* read in data */
	ndata = 0;
	if ((status = mb_datalist_open(verbose,&datalist,
					filelist,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}
	while ((status = mb_datalist_read2(verbose,datalist,
			&pstatus,path,ppath,&format,&file_weight,&error))
			== MB_SUCCESS)
		{
		ndatafile = 0;

		/* if format > 0 then input is swath sonar file */
		if (format > 0 && path[0] != '#')
		{
		/* apply pstatus */
		if (pstatus == MB_PROCESSED_USE)
			strcpy(file, ppath);
		else
			strcpy(file, path);

		/* check for mbinfo file - get file bounds if possible */
		rformat = format;
		strcpy(rfile,file);
		status = mb_check_info(verbose, file, lonflip, bounds,
				&file_in_bounds, &error);
		if (status == MB_FAILURE)
			{
			file_in_bounds = MB_YES;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* initialize the swath sonar file */
		if (file_in_bounds == MB_YES)
		    {
		    /* check for "fast bathymetry" or "fbt" file */
		    if (datatype == MBGRID_DATA_TOPOGRAPHY
			    || datatype == MBGRID_DATA_BATHYMETRY)
			{
			mb_get_fbt(verbose, rfile, &rformat, &error);
			}

		    /* call mb_read_init() */
		    if ((status = mb_read_init(
			verbose,rfile,rformat,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&mbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
			fprintf(outfp,"\nMultibeam File <%s> not initialized for reading\n",rfile);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		    /* allocate memory for reading data arrays */
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(char), (void **)&beamflag, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bath, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&amp, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathlon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathlat, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&ss, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslat, &error);

		    /* if error initializing memory then quit */
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		    /* loop over reading */
		    while (error <= MB_ERROR_NO_ERROR)
			{
			status = mb_read(verbose,mbio_ptr,&kind,
				&rpings,time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&beams_bath,&beams_amp,&pixels_ss,
				beamflag,bath,amp,bathlon,bathlat,
				ss,sslon,sslat,
				comment,&error);

			/* time gaps are not a problem here */
			if (error == MB_ERROR_TIME_GAP)
				{
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
				}

			/* print debug statements */
			if (verbose >= 2)
				{
				fprintf(outfp,"\ndbg2  Ping read in program <%s>\n",program_name);
				fprintf(outfp,"dbg2       kind:           %d\n",kind);
				fprintf(outfp,"dbg2       beams_bath:     %d\n",beams_bath);
				fprintf(outfp,"dbg2       beams_amp:      %d\n",beams_amp);
				fprintf(outfp,"dbg2       pixels_ss:      %d\n",pixels_ss);
				fprintf(outfp,"dbg2       error:          %d\n",error);
				fprintf(outfp,"dbg2       status:         %d\n",status);
				}

			if ((datatype == MBGRID_DATA_BATHYMETRY
				|| datatype == MBGRID_DATA_TOPOGRAPHY)
				&& error == MB_ERROR_NO_ERROR)
			  {

			  /* reproject beam positions if necessary */
			  if (use_projection == MB_YES)
			    {
			    for (ib=0;ib<beams_bath;ib++)
			      if (mb_beam_ok(beamflag[ib]))
				mb_proj_forward(verbose, pjptr,
						bathlon[ib], bathlat[ib],
						&bathlon[ib], &bathlat[ib],
						&error);
			    }

			  /* deal with data */
			  for (ib=0;ib<beams_bath;ib++)
			    if (mb_beam_ok(beamflag[ib]))
			      {
			      ix = (bathlon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] + 0.5*dy)/dy;
			      if (ix >= 0 && ix < gxdim
				&& iy >= 0 && iy < gydim)
			        {
			        /* check if within allowed time */
				kgrid = ix*gydim + iy;
			        if (check_time == MB_NO)
			          time_ok = MB_YES;
			        else
			          {
				  if (firsttime[kgrid] <= 0.0)
				    {
				    firsttime[kgrid] = time_d;
				    time_ok = MB_YES;
				    }
				  else if (fabs(time_d - firsttime[kgrid])
				    > timediff)
				    {
				    if (first_in_stays == MB_YES)
					time_ok = MB_NO;
				    else
					{
					time_ok = MB_YES;
					firsttime[kgrid] = time_d;
					ndata = ndata - cnt[kgrid];
					ndatafile = ndatafile - cnt[kgrid];
					cnt[kgrid] = 0;
					}
				    }
			          else
				    time_ok = MB_YES;
				  }

				/* make sure there is space for the data */
				if (time_ok == MB_YES
				  && cnt[kgrid] >= num[kgrid])
				  {
				  num[kgrid] += REALLOC_STEP_SIZE;
				  if ((data[kgrid] = (double *)
					realloc(data[kgrid], num[kgrid]*sizeof(double)))
					== NULL)
					{
					error = MB_ERROR_MEMORY_FAIL;
					mb_error(verbose,error,&message);
					fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
					    message);
					fprintf(outfp,"The weighted mean algorithm uses much less\n");
					fprintf(outfp,"memory than the median filter algorithm.\n");
					fprintf(outfp,"You could also try using ping averaging to\n");
					fprintf(outfp,"reduce the number of data points to be gridded.\n");
					fprintf(outfp,"\nProgram <%s> Terminated\n",
					    program_name);
					mb_memory_clear(verbose, &error);
					exit(error);
					}
				  }

				/* process it */
				if (time_ok == MB_YES)
				  {
				  value = data[kgrid];
				  value[cnt[kgrid]] = topofactor*bath[ib];
				  cnt[kgrid]++;
				  ndata++;
				  ndatafile++;
				  }
				}
			      }
			  }
			else if (datatype == MBGRID_DATA_AMPLITUDE
				&& error == MB_ERROR_NO_ERROR)
			  {

			  /* reproject beam positions if necessary */
			  if (use_projection == MB_YES)
			    {
			    for (ib=0;ib<beams_amp;ib++)
			      if (mb_beam_ok(beamflag[ib]))
				mb_proj_forward(verbose, pjptr,
						bathlon[ib], bathlat[ib],
						&bathlon[ib], &bathlat[ib],
						&error);
			    }

			  /* deal with data */
			  for (ib=0;ib<beams_bath;ib++)
			    if (mb_beam_ok(beamflag[ib]))
			      {
			      ix = (bathlon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] + 0.5*dy)/dy;
			      if (ix >= 0 && ix < gxdim
				&& iy >= 0 && iy < gydim)
			        {
			        /* check if within allowed time */
				kgrid = ix*gydim + iy;
			        if (check_time == MB_NO)
			          time_ok = MB_YES;
			        else
			          {
				  if (firsttime[kgrid] <= 0.0)
				    {
				    firsttime[kgrid] = time_d;
				    time_ok = MB_YES;
				    }
				  else if (fabs(time_d - firsttime[kgrid])
				    > timediff)
				    {
				    if (first_in_stays == MB_YES)
					time_ok = MB_NO;
				    else
					{
					time_ok = MB_YES;
					firsttime[kgrid] = time_d;
					ndata = ndata - cnt[kgrid];
					ndatafile = ndatafile - cnt[kgrid];
					cnt[kgrid] = 0;
					}
				    }
			          else
				    time_ok = MB_YES;
				  }

				/* make sure there is space for the data */
				if (time_ok == MB_YES
				  && cnt[kgrid] >= num[kgrid])
				  {
				  num[kgrid] += REALLOC_STEP_SIZE;
				  if ((data[kgrid] = (double *)
					realloc(data[kgrid], num[kgrid]*sizeof(double)))
					== NULL)
					{
					error = MB_ERROR_MEMORY_FAIL;
					mb_error(verbose,error,&message);
					fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
					    message);
					fprintf(outfp,"The weighted mean algorithm uses much less\n");
					fprintf(outfp,"memory than the median filter algorithm.\n");
					fprintf(outfp,"You could also try using ping averaging to\n");
					fprintf(outfp,"reduce the number of data points to be gridded.\n");
					fprintf(outfp,"\nProgram <%s> Terminated\n",
					    program_name);
					mb_memory_clear(verbose, &error);
					exit(error);
					}
				  }

				/* process it */
				if (time_ok == MB_YES)
				  {
				  value = data[kgrid];
				  value[cnt[kgrid]] = amp[ib];
				  cnt[kgrid]++;
				  ndata++;
				  ndatafile++;
				  }
				}
			      }
			  }
			else if (datatype == MBGRID_DATA_SIDESCAN
				&& error == MB_ERROR_NO_ERROR)
			  {

			  /* reproject pixel positions if necessary */
			  if (use_projection == MB_YES)
			    {
			    for (ib=0;ib<pixels_ss;ib++)
			      if (ss[ib] > MB_SIDESCAN_NULL)
				mb_proj_forward(verbose, pjptr,
						sslon[ib], sslat[ib],
						&sslon[ib], &sslat[ib],
						&error);
			    }

			  /* deal with data */
			  for (ib=0;ib<pixels_ss;ib++)
			    if (ss[ib] > MB_SIDESCAN_NULL)
			      {
			      ix = (sslon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (sslat[ib] - wbnd[2] + 0.5*dy)/dy;
			      if (ix >= 0 && ix < gxdim
				&& iy >= 0 && iy < gydim)
			        {
			        /* check if within allowed time */
				kgrid = ix*gydim + iy;
			        if (check_time == MB_NO)
			          time_ok = MB_YES;
			        else
			          {
				  if (firsttime[kgrid] <= 0.0)
				    {
				    firsttime[kgrid] = time_d;
				    time_ok = MB_YES;
				    }
				  else if (fabs(time_d - firsttime[kgrid])
				    > timediff)
				    {
				    if (first_in_stays == MB_YES)
					time_ok = MB_NO;
				    else
					{
					time_ok = MB_YES;
					firsttime[kgrid] = time_d;
					ndata = ndata - cnt[kgrid];
					ndatafile = ndatafile - cnt[kgrid];
					cnt[kgrid] = 0;
					}
				    }
			          else
				    time_ok = MB_YES;
				  }

				/* make sure there is space for the data */
				if (time_ok == MB_YES
				  && cnt[kgrid] >= num[kgrid])
				  {
				  num[kgrid] += REALLOC_STEP_SIZE;
				  if ((data[kgrid] = (double *)
					realloc(data[kgrid], num[kgrid]*sizeof(double)))
					== NULL)
					{
					error = MB_ERROR_MEMORY_FAIL;
					mb_error(verbose,error,&message);
					fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
					    message);
					fprintf(outfp,"The weighted mean algorithm uses much less\n");
					fprintf(outfp,"memory than the median filter algorithm.\n");
					fprintf(outfp,"You could also try using ping averaging to\n");
					fprintf(outfp,"reduce the number of data points to be gridded.\n");
					fprintf(outfp,"\nProgram <%s> Terminated\n",
					    program_name);
					mb_memory_clear(verbose, &error);
					exit(error);
					}
				  }

				/* process it */
				if (time_ok == MB_YES)
				  {
				  value = data[kgrid];
				  value[cnt[kgrid]] = ss[ib];
				  cnt[kgrid]++;
				  ndata++;
				  ndatafile++;
				  }
				}
			      }
			  }
			}
		    status = mb_close(verbose,&mbio_ptr,&error);
		    status = MB_SUCCESS;
		    error = MB_ERROR_NO_ERROR;
		    }
		if (verbose >= 2)
			fprintf(outfp,"\n");
		if (verbose > 0 || file_in_bounds == MB_YES)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,rfile);

		/* add to datalist if data actually contributed */
		if (ndatafile > 0 && dfp != NULL)
			{
			if (pstatus == MB_PROCESSED_USE)
				fprintf(dfp, "P:");
			else
				fprintf(dfp, "R:");
			fprintf(dfp, "%s %d %f\n", path, format, file_weight);
			fflush(dfp);
			}
		} /* end if (format > 0) */

		/* if format == 0 then input is lon,lat,values triples file */
		else if (format == 0 && path[0] != '#')
		{
		/* open data file */
		if ((rfp = fopen(path,"r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(outfp,"\nUnable to open lon,lat,value triples data path: %s\n",
				path);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		/* loop over reading */
		while (fscanf(rfp,"%lf %lf %lf",&tlon,&tlat,&tvalue) != EOF)
			{
			  /* reproject data positions if necessary */
			  if (use_projection == MB_YES)
				mb_proj_forward(verbose, pjptr,
						tlon, tlat,
						&tlon, &tlat,
						&error);

			  /* get position in grid */
			  ix = (tlon - wbnd[0] + 0.5*dx)/dx;
			  iy = (tlat - wbnd[2] + 0.5*dy)/dy;
			  if (ix >= 0 && ix < gxdim
			    && iy >= 0 && iy < gydim)
			    {
			    /* check if overwriting */
			    kgrid = ix*gydim + iy;
			    if (check_time == MB_NO)
			      time_ok = MB_YES;
			    else
			      {
			      if (firsttime[kgrid] > 0.0)
				time_ok = MB_NO;
			      else
				time_ok = MB_YES;
			      }

			    /* make sure there is space for the data */
			    if (time_ok == MB_YES
			      && cnt[kgrid] >= num[kgrid])
			      {
			      num[kgrid] += REALLOC_STEP_SIZE;
			      if ((data[kgrid] = (double *)
				    realloc(data[kgrid], num[kgrid]*sizeof(double)))
				    == NULL)
				    {
				    error = MB_ERROR_MEMORY_FAIL;
				    mb_error(verbose,error,&message);
				    fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
					message);
				    fprintf(outfp,"The weighted mean algorithm uses much less\n");
				    fprintf(outfp,"memory than the median filter algorithm.\n");
				    fprintf(outfp,"You could also try using ping averaging to\n");
				    fprintf(outfp,"reduce the number of data points to be gridded.\n");
				    fprintf(outfp,"\nProgram <%s> Terminated\n",
					program_name);
				    mb_memory_clear(verbose, &error);
				    exit(error);
				    }
			      }

			    /* process it */
			    if (time_ok == MB_YES)
			      {
			      value = data[kgrid];
			      value[cnt[kgrid]] = topofactor*tvalue;
			      cnt[kgrid]++;
			      ndata++;
			      ndatafile++;
			      }
			    }
			}
		fclose(rfp);
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		if (verbose >= 2)
			fprintf(outfp,"\n");
		if (verbose > 0)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,file);

		/* add to datalist if data actually contributed */
		if (ndatafile > 0 && dfp != NULL)
			{
			if (pstatus == MB_PROCESSED_USE)
				fprintf(dfp, "P:");
			else
				fprintf(dfp, "R:");
			fprintf(dfp, "%s %d %f\n", path, format, file_weight);
			fflush(dfp);
			}
		} /* end if (format == 0) */

		}
	if (datalist != NULL)
		mb_datalist_close(verbose,&datalist,&error);
	if (verbose > 0)
		fprintf(outfp,"\n%d total data points processed\n",ndata);

	/* now loop over all points in the output grid */
	if (verbose >= 1)
		fprintf(outfp,"\nMaking raw grid...\n");
	nbinset = 0;
	nbinzero = 0;
	nbinspline = 0;
	nbinbackground = 0;
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			if (cnt[kgrid] > 0)
				{
				value = data[kgrid];
				qsort((char *)value,cnt[kgrid],sizeof(double),
					(void *)mb_double_compare);
				if (grid_mode == MBGRID_MEDIAN_FILTER)
					{
					grid[kgrid] = value[cnt[kgrid]/2];
					}
				else if (grid_mode == MBGRID_MINIMUM_FILTER)
					{
					grid[kgrid] = value[0];
					}
				else if (grid_mode == MBGRID_MAXIMUM_FILTER)
					{
					grid[kgrid] = value[cnt[kgrid]-1];
					}
				sigma[kgrid] = 0.0;
				for (k=0;k<cnt[kgrid];k++)
					sigma[kgrid] +=
						(value[k] - grid[kgrid])
						*(value[k] - grid[kgrid]);
				if (cnt[kgrid] > 1)
					sigma[kgrid] = sqrt(sigma[kgrid]
						/(cnt[kgrid]-1));
				else
					sigma[kgrid] = 0.0;
				nbinset++;
				}
			else
				grid[kgrid] = clipvalue;
/*			fprintf(outfp,"%d %d %d  %f %f %d %f %f\n",
				i,j,kgrid,
				wbnd[0] + i*dx, wbnd[2] + j*dy,
				cnt[kgrid],grid[kgrid],sigma[kgrid]);*/
			}

	/* now deallocate space for the data */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			if (cnt[kgrid] > 0)
				free(data[kgrid]);
			}

	/***** end of median filter gridding *****/
	}

	/* close datalist if necessary */
	if (dfp != NULL)
		fclose(dfp);

	/* if clip set do smooth interpolation */
	if (clipmode != MBGRID_INTERP_NONE && clip > 0 && nbinset > 0)
		{
		/* set up data vector */
		if (setborder == MB_YES)
			ndata = 2*gxdim + 2*gydim - 2;
                else
                        ndata = 8;
		for (i=0;i<gxdim;i++)
			for (j=0;j<gydim;j++)
				{
				kgrid = i * gydim + j;
				if (grid[kgrid] < clipvalue) ndata++;
				}

#ifdef USESURFACE
		/* allocate and initialize sgrid */
		status = mb_mallocd(verbose,__FILE__,__LINE__,ndata*sizeof(float),(void **)&sxdata,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,ndata*sizeof(float),(void **)&sydata,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,ndata*sizeof(float),(void **)&szdata,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(float),(void **)&sgrid,&error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
			fprintf(outfp,"\nMBIO Error allocating interpolation work arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		memset((char *)sgrid,0,gxdim*gydim*sizeof(float));
		memset((char *)sxdata,0,ndata*sizeof(float));
		memset((char *)sydata,0,ndata*sizeof(float));
		memset((char *)szdata,0,ndata*sizeof(float));

		/* get points from grid */
                /* simultaneously find the depth values nearest to the grid corners and edge midpoints */
                ndata = 0;
		for (i=0;i<gxdim;i++)
			for (j=0;j<gydim;j++)
				{
				kgrid = i * gydim + j;
				if (grid[kgrid] < clipvalue)
					{
					sxdata[ndata] = (float)(wbnd[0] + dx*i - bdata_origin_x);
					sydata[ndata] = (float)(wbnd[2] + dy*j - bdata_origin_y);
					szdata[ndata] = (float)grid[kgrid];
					ndata++;
					}
				}

		/* if desired set border */
		if (setborder == MB_YES)
			{
			for (i=0;i<gxdim;i++)
				{
				j = 0;
				kgrid = i * gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sxdata[ndata] = (float)(wbnd[0] + dx*i - bdata_origin_x);
					sydata[ndata] = (float)(wbnd[2] + dy*j - bdata_origin_y);
					szdata[ndata] = (float)border;
					ndata++;
					}
				j = gydim - 1;
				kgrid = i * gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sxdata[ndata] = (float)(wbnd[0] + dx*i - bdata_origin_x);
					sydata[ndata] = (float)(wbnd[2] + dy*j - bdata_origin_y);
					szdata[ndata] = (float)border;
					ndata++;
					}
				}
			for (j=1;j<gydim-1;j++)
				{
				i = 0;
				kgrid = i * gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sxdata[ndata] = (float)(wbnd[0] + dx*i - bdata_origin_x);
					sydata[ndata] = (float)(wbnd[2] + dy*j - bdata_origin_y);
					szdata[ndata] = (float)border;
					ndata++;
					}
				i = gxdim - 1;
				kgrid = i * gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sxdata[ndata] = (float)(wbnd[0] + dx*i - bdata_origin_x);
					sydata[ndata] = (float)(wbnd[2] + dy*j - bdata_origin_y);
					szdata[ndata] = (float)border;
					ndata++;
					}
				}
			}

		/* do the interpolation */
		fprintf(outfp,"\nDoing Surface spline interpolation with %d data points...\n",ndata);
		mb_surface(verbose, ndata, sxdata, sydata, szdata,
			(float)(gbnd[0] - bdata_origin_x), (float)(gbnd[1] - bdata_origin_x),
                        (float)(gbnd[2] - bdata_origin_y), (float)(gbnd[3] - bdata_origin_y),
                        dx, dy,
			tension, sgrid);
#else
		/* allocate and initialize sgrid */
		status = mb_mallocd(verbose,__FILE__,__LINE__,3*ndata*sizeof(float),(void **)&sdata,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(float),(void **)&sgrid,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,ndata*sizeof(float),(void **)&work1,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,ndata*sizeof(int),(void **)&work2,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,(gxdim+gydim)*sizeof(int),(void **)&work3,&error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
			fprintf(outfp,"\nMBIO Error allocating interpolation work arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		memset((char *)sgrid,0,gxdim*gydim*sizeof(float));
		memset((char *)sdata,0,3*ndata*sizeof(float));
		memset((char *)work1,0,ndata*sizeof(float));
		memset((char *)work2,0,ndata*sizeof(int));
		memset((char *)work3,0,(gxdim+gydim)*sizeof(int));

		/* get points from grid */
                /* simultaneously find the depth values nearest to the grid corners and edge midpoints */
                ndata = 0;
 		for (i=0;i<gxdim;i++)
			for (j=0;j<gydim;j++)
				{
				kgrid = i * gydim + j;
				if (grid[kgrid] < clipvalue)
					{
					sdata[ndata++] = (float)(wbnd[0] + dx*i - bdata_origin_x);
					sdata[ndata++] = (float)(wbnd[2] + dy*j - bdata_origin_y);
					sdata[ndata++] = (float)grid[kgrid];
					}
				}

		/* if desired set border */
		if (setborder == MB_YES)
			{
			for (i=0;i<gxdim;i++)
				{
				j = 0;
				kgrid = i * gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sdata[ndata++] = (float)(wbnd[0] + dx*i - bdata_origin_x);
					sdata[ndata++] = (float)(wbnd[2] + dy*j - bdata_origin_y);
					sdata[ndata++] = (float)border;
					}
				j = gydim - 1;
				kgrid = i * gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sdata[ndata++] = (float)(wbnd[0] + dx*i - bdata_origin_x);
					sdata[ndata++] = (float)(wbnd[2] + dy*j - bdata_origin_y);
					sdata[ndata++] = (float)border;
					}
				}
			for (j=1;j<gydim-1;j++)
				{
				i = 0;
				kgrid = i * gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sdata[ndata++] = (float)(wbnd[0] + dx*i - bdata_origin_x);
					sdata[ndata++] = (float)(wbnd[2] + dy*j - bdata_origin_y);
					sdata[ndata++] = (float)border;
					}
				i = gxdim - 1;
				kgrid = i * gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sdata[ndata++] = (float)(wbnd[0] + dx*i - bdata_origin_x);
					sdata[ndata++] = (float)(wbnd[2] + dy*j - bdata_origin_y);
					sdata[ndata++] = (float)border;
					}
				}
			}
                ndata = ndata/3;

		/* do the interpolation */
		cay = (float)tension;
		xmin = (float)(wbnd[0] - 0.5 * dx - bdata_origin_x);
		ymin = (float)(wbnd[2] - 0.5 * dy - bdata_origin_y);
		ddx = (float)dx;
		ddy = (float)dy;
		fprintf(outfp,"\nDoing Zgrid spline interpolation with %d data points...\n",ndata);
/*for (i=0;i<ndata/3;i++)
{
if (sdata[3*i+2]>2000.0)
fprintf(stderr,"%d %f\n",i,sdata[3*i+2]);
}*/
		if (clipmode == MBGRID_INTERP_ALL)
			clip = MAX(gxdim,gydim);
		mb_zgrid(sgrid,&gxdim,&gydim,&xmin,&ymin,
			&ddx,&ddy,sdata,&ndata,
			work1,work2,work3,&cay,&clip);
#endif

		if (clipmode == MBGRID_INTERP_GAP)
		    fprintf(outfp,"Applying spline interpolation to fill gaps of %d cells or less...\n",clip);
		else if (clipmode == MBGRID_INTERP_NEAR)
		    fprintf(outfp,"Applying spline interpolation to fill %d cells from data...\n",clip);
		else if (clipmode == MBGRID_INTERP_ALL)
		    fprintf(outfp,"Applying spline interpolation to fill all undefined cells in the grid...\n");

		/* translate the interpolation into the grid array
		    filling only data gaps */
		zflag = 5.0e34;
		if (clipmode == MBGRID_INTERP_GAP)
			{
			for (i=0;i<gxdim;i++)
			    for (j=0;j<gydim;j++)
				{
				kgrid = i * gydim + j;
#ifdef USESURFACE
				kint = i + (gydim -j - 1) * gxdim;
#else
				kint = i + j*gxdim;
#endif
				num[kgrid] = MB_NO;
				if (grid[kgrid] >= clipvalue
				    && sgrid[kint] < zflag)
				    {
				    /* initialize direction mask of search */
				    for (ii=0;ii<9;ii++)
					dmask[ii] = MB_NO;

				    /* loop over rings around point, starting close */
				    for (ir=0; ir <= clip && num[kgrid] == MB_NO; ir++)
				      {
				      /* set bounds of search */
				      i1 = MAX(0, i - ir);
				      i2 = MIN(gxdim - 1, i + ir);
				      j1 = MAX(0, j - ir);
				      j2 = MIN(gydim - 1, j + ir);

				      jj = j1;
				      for (ii=i1;ii<=i2 && num[kgrid] == MB_NO;ii++)
				        {
					if (grid[ii*gydim+jj] < clipvalue)
					    {
					    r = sqrt((double)((ii-i)*(ii-i) + (jj-j)*(jj-j)));
					    iii = rint((ii - i)/r) + 1;
					    jjj = rint((jj - j)/r) + 1;
					    kkk = iii * 3 + jjj;
					    dmask[kkk] = MB_YES;
					    if ((dmask[0] && dmask[8])
						|| (dmask[3] && dmask[5])
						|| (dmask[6] && dmask[2])
						|| (dmask[1] && dmask[7]))
						num[kgrid] = MB_YES;
					    }
					}

				      jj = j2;
				      for (ii=i1;ii<=i2 && num[kgrid] == MB_NO;ii++)
				        {
					if (grid[ii*gydim+jj] < clipvalue)
					    {
					    r = sqrt((double)((ii-i)*(ii-i) + (jj-j)*(jj-j)));
					    iii = rint((ii - i)/r) + 1;
					    jjj = rint((jj - j)/r) + 1;
					    kkk = iii * 3 + jjj;
					    dmask[kkk] = MB_YES;
					    if ((dmask[0] && dmask[8])
						|| (dmask[3] && dmask[5])
						|| (dmask[6] && dmask[2])
						|| (dmask[1] && dmask[7]))
						num[kgrid] = MB_YES;
					    }
					}

				      ii = i1;
				      for (jj=j1;jj<=j2 && num[kgrid] == MB_NO;jj++)
				        {
					if (grid[ii*gydim+jj] < clipvalue)
					    {
					    r = sqrt((double)((ii-i)*(ii-i) + (jj-j)*(jj-j)));
					    iii = rint((ii - i)/r) + 1;
					    jjj = rint((jj - j)/r) + 1;
					    kkk = iii * 3 + jjj;
					    dmask[kkk] = MB_YES;
					    if ((dmask[0] && dmask[8])
						|| (dmask[3] && dmask[5])
						|| (dmask[6] && dmask[2])
						|| (dmask[1] && dmask[7]))
						num[kgrid] = MB_YES;
					    }
					}

				      ii = i2;
				      for (jj=j1;jj<=j2 && num[kgrid] == MB_NO;jj++)
				        {
					if (grid[ii*gydim+jj] < clipvalue)
					    {
					    r = sqrt((double)((ii-i)*(ii-i) + (jj-j)*(jj-j)));
					    iii = rint((ii - i)/r) + 1;
					    jjj = rint((jj - j)/r) + 1;
					    kkk = iii * 3 + jjj;
					    dmask[kkk] = MB_YES;
					    if ((dmask[0] && dmask[8])
						|| (dmask[3] && dmask[5])
						|| (dmask[6] && dmask[2])
						|| (dmask[1] && dmask[7]))
						num[kgrid] = MB_YES;
					    }
					}
				      }
				    }
				}
			for (i=0;i<gxdim;i++)
			    for (j=0;j<gydim;j++)
				{
				kgrid = i * gydim + j;
#ifdef USESURFACE
				kint = i + (gydim -j - 1) * gxdim;
#else
				kint = i + j*gxdim;
#endif
				if (num[kgrid] == MB_YES)
					{
					grid[kgrid] = sgrid[kint];
					nbinspline++;
					}
				}
			}

		/* translate the interpolation into the grid array
		    filling by proximity */
		else if (clipmode == MBGRID_INTERP_NEAR)
			{
			for (i=0;i<gxdim;i++)
			    for (j=0;j<gydim;j++)
				{
				kgrid = i * gydim + j;
#ifdef USESURFACE
				kint = i + (gydim -j - 1) * gxdim;
#else
				kint = i + j*gxdim;
#endif

				num[kgrid] = MB_NO;
				if (grid[kgrid] >= clipvalue
				    && sgrid[kint] < zflag)
				    {
				    /* loop over rings around point, starting close */
				    for (ir=0; ir <= clip && num[kgrid] == MB_NO; ir++)
				      {
				      /* set bounds of search */
				      i1 = MAX(0, i - ir);
				      i2 = MIN(gxdim - 1, i + ir);
				      j1 = MAX(0, j - ir);
				      j2 = MIN(gydim - 1, j + ir);

				      jj = j1;
				      for (ii=i1;ii<=i2 && num[kgrid] == MB_NO;ii++)
				        {
					if (grid[ii*gydim+jj] < clipvalue)
					    {
					    num[kgrid] = MB_YES;
					    }
					}

				      jj = j2;
				      for (ii=i1;ii<=i2 && num[kgrid] == MB_NO;ii++)
				        {
					if (grid[ii*gydim+jj] < clipvalue)
					    {
					    num[kgrid] = MB_YES;
					    }
					}

				      ii = i1;
				      for (jj=j1;jj<=j2 && num[kgrid] == MB_NO;jj++)
				        {
					if (grid[ii*gydim+jj] < clipvalue)
					    {
					    num[kgrid] = MB_YES;
					    }
					}

				      ii = i2;
				      for (jj=j1;jj<=j2 && num[kgrid] == MB_NO;jj++)
				        {
					if (grid[ii*gydim+jj] < clipvalue)
					    {
					    num[kgrid] = MB_YES;
					    }
					}
				      }
				    }
				}
			for (i=0;i<gxdim;i++)
			    for (j=0;j<gydim;j++)
				{
				kgrid = i * gydim + j;
#ifdef USESURFACE
				kint = i + (gydim -j - 1) * gxdim;
#else
				kint = i + j*gxdim;
#endif
				if (num[kgrid] == MB_YES)
					{
					grid[kgrid] = sgrid[kint];
					nbinspline++;
					}
				}
			}

		/* translate the interpolation into the grid array
		    filling all empty bins */
		else
			{
			for (i=0;i<gxdim;i++)
			    for (j=0;j<gydim;j++)
				{
				kgrid = i * gydim + j;
#ifdef USESURFACE
				kint = i + (gydim -j - 1) * gxdim;
#else
				kint = i + j*gxdim;
#endif
				if (grid[kgrid] >= clipvalue
				    && sgrid[kint] < zflag)
					{
					grid[kgrid] = sgrid[kint];
					nbinspline++;
					}
				}
			}

		/* deallocate the interpolation arrays */
#ifdef USESURFACE
		mb_freed(verbose,__FILE__,__LINE__,(void **)&sxdata,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&sydata,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&szdata,&error);
#else
		mb_freed(verbose,__FILE__,__LINE__,(void **)&sdata,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&work1,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&work2,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&work3,&error);
#endif
		mb_freed(verbose,__FILE__,__LINE__,(void **)&sgrid,&error);
		}

	/* if grdrasterid set and background data previously read in
		then interpolate it onto internal grid */
	if (grdrasterid != 0 && nbackground > 0)
		{

		/* allocate and initialize grid and work arrays */
#ifdef USESURFACE
		status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(float),(void **)&sgrid,&error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
			fprintf(outfp,"\nMBIO Error allocating background data array:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		memset((char *)sgrid,0,gxdim*gydim*sizeof(float));
#else
		status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(float),(void **)&sgrid,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,nbackground*sizeof(float),(void **)&work1,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,nbackground*sizeof(int),(void **)&work2,&error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose,__FILE__,__LINE__,(gxdim+gydim)*sizeof(int),(void **)&work3,&error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
			fprintf(outfp,"\nMBIO Error allocating background interpolation work arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		memset((char *)sgrid,0,gxdim*gydim*sizeof(float));
		memset((char *)work1,0,nbackground*sizeof(float));
		memset((char *)work2,0,nbackground*sizeof(int));
		memset((char *)work3,0,(gxdim+gydim)*sizeof(int));
#endif

		/* do the interpolation */
		fprintf(outfp,"\nDoing spline interpolation with %d data points from background...\n",nbackground);
#ifdef USESURFACE
		mb_surface(verbose, nbackground, bxdata, bydata, bzdata,
			(float)(wbnd[0] - bdata_origin_x), (float)(wbnd[1] - bdata_origin_x),
                        (float)(wbnd[2] - bdata_origin_y), (float)(wbnd[3] - bdata_origin_y),
                        dx, dy,
			tension, sgrid);
#else
		cay = (float)tension;
		xmin = (float)(wbnd[0] - 0.5 * dx - bdata_origin_x);
		ymin = (float)(wbnd[2] - 0.5 * dy - bdata_origin_y);
		ddx = (float)dx;
		ddy = (float)dy;
		clip = MAX(gxdim,gydim);
		fprintf(outfp,"\nDoing Zgrid spline interpolation with %d background points...\n",nbackground);
		mb_zgrid2(sgrid,&gxdim,&gydim,&xmin,&ymin,
			&ddx,&ddy,bdata,&nbackground,
			work1,work2,work3,&cay,&clip);
#endif

		/* translate the interpolation into the grid array
		    - interpolate only to fill a data gap */
		zflag = 5.0e34;
		for (i=0;i<gxdim;i++)
		    for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
#ifdef USESURFACE
			kint = i + (gydim -j - 1) * gxdim;
#else
			kint = i + j*gxdim;
#endif
			if (grid[kgrid] >= clipvalue
			    && sgrid[kint] < zflag)
				{
				grid[kgrid] = sgrid[kint];
				nbinbackground++;
				}
			}
#ifdef USESURFACE
		mb_freed(verbose,__FILE__,__LINE__,(void **)&bxdata,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&bydata,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&bzdata,&error);
#else
		mb_freed(verbose,__FILE__,__LINE__,(void **)&bdata,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&work1,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&work2,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&work3,&error);
#endif
		mb_freed(verbose,__FILE__,__LINE__,(void **)&sgrid,&error);
		}

	/* get min max of data */
	zclip = clipvalue;
	zmin = zclip;
	zmax = zclip;
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			if (zmin == zclip
				&& grid[kgrid] < zclip)
				zmin = grid[kgrid];
			if (zmax == zclip
				&& grid[kgrid] < zclip)
				zmax = grid[kgrid];
			if (grid[kgrid] < zmin && grid[kgrid] < zclip)
				zmin = grid[kgrid];
			if (grid[kgrid] > zmax && grid[kgrid] < zclip)
				zmax = grid[kgrid];
			}
	if (zmin == zclip)
		zmin = 0.0;
	if (zmax == zclip)
		zmax = 0.0;

	/* get min max of data distribution */
	nmax = 0;
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			if (cnt[kgrid] > nmax)
				nmax = cnt[kgrid];
			}

	/* get min max of standard deviation */
	smin = 0.0;
	smax = 0.0;
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i * gydim + j;
			if (smin == 0.0
				&& cnt[kgrid] > 0)
				smin = sigma[kgrid];
			if (smax == 0.0
				&& cnt[kgrid] > 0)
				smax = sigma[kgrid];
			if (sigma[kgrid] < smin && cnt[kgrid] > 0)
				smin = sigma[kgrid];
			if (sigma[kgrid] > smax && cnt[kgrid] > 0)
				smax = sigma[kgrid];
			}
	nbinzero = gxdim*gydim - nbinset - nbinspline - nbinbackground;
	fprintf(outfp,"\nTotal number of bins:            %d\n",gxdim*gydim);
	fprintf(outfp,"Bins set using data:             %d\n",nbinset);
	fprintf(outfp,"Bins set using interpolation:    %d\n",nbinspline);
	fprintf(outfp,"Bins set using background:       %d\n",nbinbackground);
	fprintf(outfp,"Bins not set:                    %d\n",nbinzero);
	fprintf(outfp,"Maximum number of data in a bin: %d\n",nmax);
	fprintf(outfp,"Minimum value: %10.2f   Maximum value: %10.2f\n",
		zmin,zmax);
	fprintf(outfp,"Minimum sigma: %10.5f   Maximum sigma: %10.5f\n",
		smin,smax);

	/* write first output file */
	if (verbose > 0)
		fprintf(outfp,"\nOutputting results...\n");
	for (i=0;i<xdim;i++)
		for (j=0;j<ydim;j++)
			{
			kgrid = (i + offx)*gydim + (j + offy);
			kout = i*ydim + j;
			output[kout] = (float) grid[kgrid];
			if (gridkind != MBGRID_ASCII
				&& gridkind != MBGRID_ARCASCII
				&& grid[kgrid] >= clipvalue)
				{
				output[kout] = outclipvalue;
				}
			}
	if (gridkind == MBGRID_ASCII)
		{
		strcpy(ofile,fileroot);
		strcat(ofile,".asc");
		status = write_ascii(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],
			dx,dy,&error);
		}
	else if (gridkind == MBGRID_ARCASCII)
		{
		strcpy(ofile,fileroot);
		strcat(ofile,".asc");
		status = write_arcascii(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],
			dx,dy,outclipvalue,&error);
		}
	else if (gridkind == MBGRID_OLDGRD)
		{
		strcpy(ofile,fileroot);
		strcat(ofile,".grd1");
		status = write_oldgrd(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],dx,dy,&error);
		}
	else if (gridkind == MBGRID_CDFGRD)
		{
		strcpy(ofile,fileroot);
		strcat(ofile,".grd");
		status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],
			zmin,zmax,dx,dy,
			xlabel,ylabel,zlabel,title,projection_id,
			argc,argv,&error);
		}
	else if (gridkind == MBGRID_GMTGRD)
		{
		strcpy(ofile,fileroot);
		strcat(ofile,".grd");
		sprintf(ofile,"%s.grd%s", fileroot, gridkindstring);
		status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],
			zmin,zmax,dx,dy,
			xlabel,ylabel,zlabel,title,projection_id,
			argc,argv,&error);
		}
	if (status != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(outfp,"\nError writing output file: %s\n%s\n",
			ofile,message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}

	/* write second output file */
	if (more == MB_YES)
		{
		for (i=0;i<xdim;i++)
			for (j=0;j<ydim;j++)
				{
				kgrid = (i + offx)*gydim + (j + offy);
				kout = i*ydim + j;
				output[kout] = (float) cnt[kgrid];
				if (output[kout] < 0.0)
					output[kout] = 0.0;
				if (gridkind != MBGRID_ASCII
					&& gridkind != MBGRID_ARCASCII
					&& cnt[kgrid] <= 0)
					output[kout] = outclipvalue;
				}
		if (gridkind == MBGRID_ASCII)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_num.asc");
			status = write_ascii(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBGRID_ARCASCII)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_num.asc");
			status = write_arcascii(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,outclipvalue,&error);
			}
		else if (gridkind == MBGRID_OLDGRD)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_num.grd1");
			status = write_oldgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBGRID_CDFGRD)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_num.grd");
			status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				zmin,zmax,dx,dy,
				xlabel,ylabel,nlabel,title,projection_id,
				argc,argv,&error);
			}
		else if (gridkind == MBGRID_GMTGRD)
			{
			sprintf(ofile,"%s_num.grd%s", fileroot, gridkindstring);
			status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				zmin,zmax,dx,dy,
				xlabel,ylabel,zlabel,title,projection_id,
				argc,argv,&error);
			}
		if (status != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nError writing output file: %s\n%s\n",
				ofile,message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		/* write third output file */
		for (i=0;i<xdim;i++)
			for (j=0;j<ydim;j++)
				{
				kgrid = (i + offx)*gydim + (j + offy);
				kout = i*ydim + j;
				output[kout] = (float) sigma[kgrid];
				if (output[kout] < 0.0)
					output[kout] = 0.0;
				if (gridkind != MBGRID_ASCII
					&& gridkind != MBGRID_ARCASCII
					&& cnt[kgrid] <= 0)
					output[kout] = outclipvalue;
				}
		if (gridkind == MBGRID_ASCII)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_sd.asc");
			status = write_ascii(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBGRID_ARCASCII)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_sd.asc");
			status = write_arcascii(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,outclipvalue,&error);
			}
		else if (gridkind == MBGRID_OLDGRD)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_sd.grd1");
			status = write_oldgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBGRID_CDFGRD)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_sd.grd");
			status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				zmin,zmax,dx,dy,
				xlabel,ylabel,sdlabel,title,projection_id,
				argc,argv,&error);
			}
		else if (gridkind == MBGRID_GMTGRD)
			{
			sprintf(ofile,"%s_sd.grd%s", fileroot, gridkindstring);
			status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				zmin,zmax,dx,dy,
				xlabel,ylabel,zlabel,title,projection_id,
				argc,argv,&error);
			}
		if (status != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nError writing output file: %s\n%s\n",
				ofile,message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		}

	/* deallocate arrays */
	mb_freed(verbose,__FILE__,__LINE__,(void **)&grid,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&norm,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&num,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&cnt,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&sigma,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&firsttime,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&output,&error);

	/* deallocate projection */
	if (use_projection == MB_YES)
		proj_status = mb_proj_free(verbose, &(pjptr), &error);

	/* run mbm_grdplot */
	if (gridkind == MBGRID_GMTGRD)
		{
		/* execute mbm_grdplot */
		strcpy(ofile,fileroot);
		strcat(ofile,".grd");
		if (datatype == MBGRID_DATA_BATHYMETRY)
			{
			sprintf(plot_cmd, "mbm_grdplot -I%s%s -G1 -C -D -V -L\"File %s - %s:%s\"",
				ofile, gridkindstring, ofile, title, zlabel);
			}
		else if (datatype == MBGRID_DATA_TOPOGRAPHY)
			{
			sprintf(plot_cmd, "mbm_grdplot -I%s%s -G1 -C -V -L\"File %s - %s:%s\"",
				ofile, gridkindstring, ofile, title, zlabel);
			}
		else if (datatype == MBGRID_DATA_AMPLITUDE)
			{
			sprintf(plot_cmd, "mbm_grdplot -I%s%s -G1 -W1/4 -S -D -V -L\"File %s - %s:%s\"",
				ofile, gridkindstring, ofile, title, zlabel);
			}
		else
			{
			sprintf(plot_cmd, "mbm_grdplot -I%s%s -G1 -W1/4 -S -D -V -L\"File %s - %s:%s\"",
				ofile, gridkindstring, ofile, title, zlabel);
			}
		if (verbose)
			{
			fprintf(outfp, "\nexecuting mbm_grdplot...\n%s\n",
				plot_cmd);
			}
		plot_status = system(plot_cmd);
		if (plot_status == -1)
			{
			fprintf(outfp, "\nError executing mbm_grdplot on output file %s\n", ofile);
			}
		}
	if (more == MB_YES
		&& gridkind == MBGRID_GMTGRD)
		{
		/* execute mbm_grdplot */
		strcpy(ofile,fileroot);
		strcat(ofile,"_num.grd");
		sprintf(plot_cmd, "mbm_grdplot -I%s%s -G1 -W1/2 -V -L\"File %s - %s:%s\"",
			ofile, gridkindstring, ofile, title, nlabel);
		if (verbose)
			{
			fprintf(outfp, "\nexecuting mbm_grdplot...\n%s\n",
				plot_cmd);
			}
		plot_status = system(plot_cmd);
		if (plot_status == -1)
			{
			fprintf(outfp, "\nError executing mbm_grdplot on output file grd_%s\n", fileroot);
			}

		/* execute mbm_grdplot */
		strcpy(ofile,fileroot);
		strcat(ofile,"_sd.grd");
		sprintf(plot_cmd, "mbm_grdplot -I%s%s -G1 -W1/2 -V -L\"File %s - %s:%s\"",
			ofile, gridkindstring, ofile, title, sdlabel);
		if (verbose)
			{
			fprintf(outfp, "\nexecuting mbm_grdplot...\n%s\n",
				plot_cmd);
			}
		plot_status = system(plot_cmd);
		if (plot_status == -1)
			{
			fprintf(outfp, "\nError executing mbm_grdplot on output file grd_%s\n", fileroot);
			}
		}

	if (verbose > 0)
		fprintf(outfp,"\nDone.\n\n");

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(outfp,"dbg2  Ending status:\n");
		fprintf(outfp,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
/*
 * function write_ascii writes output grid to an ascii file
 */
int write_ascii(int verbose, char *outfile, float *grid,
		int nx, int ny,
		double xmin, double xmax, double ymin, double ymax,
		double dx, double dy, int *error)
{
	char	*function_name = "write_ascii";
	int	status = MB_SUCCESS;
	FILE	*fp;
	int	i;
	time_t	right_now;
	char	date[32], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
	char	*ctime();
	char	*getenv();

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(outfp,"dbg2  Input arguments:\n");
		fprintf(outfp,"dbg2       verbose:    %d\n",verbose);
		fprintf(outfp,"dbg2       outfile:    %s\n",outfile);
		fprintf(outfp,"dbg2       grid:       %p\n",(void *)grid);
		fprintf(outfp,"dbg2       nx:         %d\n",nx);
		fprintf(outfp,"dbg2       ny:         %d\n",ny);
		fprintf(outfp,"dbg2       xmin:       %f\n",xmin);
		fprintf(outfp,"dbg2       xmax:       %f\n",xmax);
		fprintf(outfp,"dbg2       ymin:       %f\n",ymin);
		fprintf(outfp,"dbg2       ymax:       %f\n",ymax);
		fprintf(outfp,"dbg2       dx:         %f\n",dx);
		fprintf(outfp,"dbg2       dy:         %f\n",dy);
		}

	/* open the file */
	if ((fp = fopen(outfile,"w")) == NULL)
		{
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		}

	/* output grid */
	if (status == MB_SUCCESS)
		{
		fprintf(fp,"grid created by program MBGRID\n");
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
		if (user_ptr != NULL)
			strcpy(user,user_ptr);
		else
			strcpy(user, "unknown");
		i = gethostname(host,MB_PATH_MAXLINE);
		fprintf(fp,"program run by %s on %s at %s\n",user,host,date);
		fprintf(fp,"%d %d\n%f %f %f %f\n",nx,ny,xmin,xmax,ymin,ymax);
		for (i=0;i<nx*ny;i++)
			{
			fprintf(fp,"%13.5g ",grid[i]);
			if ((i+1) % 6 == 0) fprintf(fp,"\n");
			}
		if ((nx*ny) % 6 != 0) fprintf(fp,"\n");
		fclose(fp);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(outfp,"dbg2  Return values:\n");
		fprintf(outfp,"dbg2       error:      %d\n",*error);
		fprintf(outfp,"dbg2  Return status:\n");
		fprintf(outfp,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
/*
 * function write_arcascii writes output grid to an Arc/Info ascii file
 */
int write_arcascii(int verbose, char *outfile, float *grid,
		int nx, int ny,
		double xmin, double xmax, double ymin, double ymax,
		double dx, double dy, double nodata, int *error)
{
	char	*function_name = "write_ascii";
	int	status = MB_SUCCESS;
	FILE	*fp;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(outfp,"dbg2  Input arguments:\n");
		fprintf(outfp,"dbg2       verbose:    %d\n",verbose);
		fprintf(outfp,"dbg2       outfile:    %s\n",outfile);
		fprintf(outfp,"dbg2       grid:       %p\n",(void *)grid);
		fprintf(outfp,"dbg2       nx:         %d\n",nx);
		fprintf(outfp,"dbg2       ny:         %d\n",ny);
		fprintf(outfp,"dbg2       xmin:       %f\n",xmin);
		fprintf(outfp,"dbg2       xmax:       %f\n",xmax);
		fprintf(outfp,"dbg2       ymin:       %f\n",ymin);
		fprintf(outfp,"dbg2       ymax:       %f\n",ymax);
		fprintf(outfp,"dbg2       dx:         %f\n",dx);
		fprintf(outfp,"dbg2       dy:         %f\n",dy);
		fprintf(outfp,"dbg2       nodata:     %f\n",nodata);
		}

	/* open the file */
	if ((fp = fopen(outfile,"w")) == NULL)
		{
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		}

	/* output grid */
	if (status == MB_SUCCESS)
		{
		fprintf(fp, "ncols %d\n", nx);
		fprintf(fp, "nrows %d\n", ny);
		fprintf(fp, "xllcorner %.10g\n", xmin - 0.5 * dx);
		fprintf(fp, "yllcorner %.10g\n", ymin - 0.5 * dy);
		fprintf(fp, "cellsize %.10g\n", dx);
		fprintf(fp, "nodata_value -99999\n");
		for (j=0;j<ny;j++)
		    {
		    for (i=0;i<nx;i++)
			{
			k = i * ny + (ny - 1 - j);
			if (grid[k] == nodata)
			    fprintf(fp, "-99999 ");
			else
			    fprintf(fp,"%f ",grid[k]);
			}
		    fprintf(fp, "\n");
		    }
		fclose(fp);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(outfp,"dbg2  Return values:\n");
		fprintf(outfp,"dbg2       error:      %d\n",*error);
		fprintf(outfp,"dbg2  Return status:\n");
		fprintf(outfp,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
/*
 * function write_oldgrd writes output grid to a
 * GMT version 1 binary grd file
 */
int write_oldgrd(int verbose, char *outfile, float *grid,
		int nx, int ny,
		double xmin, double xmax, double ymin, double ymax,
		double dx, double dy, int *error)
{
	char	*function_name = "write_oldgrd";
	int	status = MB_SUCCESS;
	FILE	*fp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(outfp,"dbg2  Input arguments:\n");
		fprintf(outfp,"dbg2       verbose:    %d\n",verbose);
		fprintf(outfp,"dbg2       outfile:    %s\n",outfile);
		fprintf(outfp,"dbg2       grid:       %p\n",(void *)grid);
		fprintf(outfp,"dbg2       nx:         %d\n",nx);
		fprintf(outfp,"dbg2       ny:         %d\n",ny);
		fprintf(outfp,"dbg2       xmin:       %f\n",xmin);
		fprintf(outfp,"dbg2       xmax:       %f\n",xmax);
		fprintf(outfp,"dbg2       ymin:       %f\n",ymin);
		fprintf(outfp,"dbg2       ymax:       %f\n",ymax);
		fprintf(outfp,"dbg2       dx:         %f\n",dx);
		fprintf(outfp,"dbg2       dy:         %f\n",dy);
		}

	/* open the file */
	if ((fp = fopen(outfile,"w")) == NULL)
		{
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		}

	/* output grid */
	if (status == MB_SUCCESS)
		{
		fwrite ((char *)&nx, 1, 4, fp);
		fwrite ((char *)&ny, 1, 4, fp);
		fwrite ((char *)&xmin, 1, 8, fp);
		fwrite ((char *)&xmax, 1, 8, fp);
		fwrite ((char *)&ymin, 1, 8, fp);
		fwrite ((char *)&ymax, 1, 8, fp);
		fwrite ((char *)&dx, 1, 8, fp);
		fwrite ((char *)&dy, 1, 8, fp);
		fwrite ((char *)grid, nx*ny, 4, fp);
		fclose(fp);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(outfp,"dbg2  Return values:\n");
		fprintf(outfp,"dbg2       error:      %d\n",*error);
		fprintf(outfp,"dbg2  Return status:\n");
		fprintf(outfp,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
/*
 * function write_cdfgrd writes output grid to a
 * GMT version 2 netCDF grd file
 */
int write_cdfgrd(int verbose, char *outfile, float *grid,
		int nx, int ny,
		double xmin, double xmax, double ymin, double ymax,
		double zmin, double zmax, double dx, double dy,
		char *xlab, char *ylab, char *zlab, char *titl,
		char *projection, int argc, char **argv,
		int *error)
{
	char	*function_name = "write_cdfgrd";
	int	status = MB_SUCCESS;
	struct GRD_HEADER grd;
	double	w, e, s, n;
#ifdef GMT_MINOR_VERSION
	GMT_LONG	pad[4];
#else
	int	pad[4];
#endif
	float	*a;
	time_t	right_now;
	char	date[32], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
	char	remark[MB_PATH_MAXLINE];
	int	i, j, kg, ka;
	char	*message;
	char	*ctime();
	char	*getenv();

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(outfp,"dbg2  Input arguments:\n");
		fprintf(outfp,"dbg2       verbose:    %d\n",verbose);
		fprintf(outfp,"dbg2       outfile:    %s\n",outfile);
		fprintf(outfp,"dbg2       grid:       %p\n",(void *)grid);
		fprintf(outfp,"dbg2       nx:         %d\n",nx);
		fprintf(outfp,"dbg2       ny:         %d\n",ny);
		fprintf(outfp,"dbg2       xmin:       %f\n",xmin);
		fprintf(outfp,"dbg2       xmax:       %f\n",xmax);
		fprintf(outfp,"dbg2       ymin:       %f\n",ymin);
		fprintf(outfp,"dbg2       ymax:       %f\n",ymax);
		fprintf(outfp,"dbg2       dx:         %f\n",dx);
		fprintf(outfp,"dbg2       dy:         %f\n",dy);
		fprintf(outfp,"dbg2       xlab:       %s\n",xlab);
		fprintf(outfp,"dbg2       ylab:       %s\n",ylab);
		fprintf(outfp,"dbg2       zlab:       %s\n",zlab);
		fprintf(outfp,"dbg2       titl:       %s\n",titl);
		fprintf(outfp,"dbg2       argc:       %d\n",argc);
		fprintf(outfp,"dbg2       *argv:      %p\n",(void *)*argv);
		}

	/* inititialize grd header */
	GMT_program = program_name;
	GMT_grd_init (&grd, 1, argv, FALSE);
	GMT_io_init ();
	GMT_grdio_init ();
	GMT_make_fnan (GMT_f_NaN);
	GMT_make_dnan (GMT_d_NaN);

	/* copy values to grd header */
	grd.nx = nx;
	grd.ny = ny;
	grd.node_offset = 0;
	grd.x_min = xmin;
	grd.x_max = xmax;
	grd.y_min = ymin;
	grd.y_max = ymax;
	grd.z_min = zmin;
	grd.z_max = zmax;
	grd.x_inc = dx;
	grd.y_inc = dy;
	grd.z_scale_factor = 1.0;
	grd.z_add_offset = 0.0;
	strcpy(grd.x_units,xlab);
	strcpy(grd.y_units,ylab);
	strcpy(grd.z_units,zlab);
	strcpy(grd.title,titl);
	strcpy(grd.command,"\0");
	right_now = time((time_t *)0);
	strcpy(date,ctime(&right_now));
        date[strlen(date)-1] = '\0';
	if ((user_ptr = getenv("USER")) == NULL)
		user_ptr = getenv("LOGNAME");
	if (user_ptr != NULL)
		strcpy(user,user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host,MB_PATH_MAXLINE);
	sprintf(remark,"\n\tProjection: %s\n\tGrid created by %s\n\tMB-system Version %s\n\tRun by <%s> on <%s> at <%s>",
		projection,program_name,MB_VERSION,user,host,date);
	strncpy(grd.remark, remark, 159);

	/* set extract wesn,pad */
	w = 0.0;
	e = 0.0;
	s = 0.0;
	n = 0.0;
	for (i=0;i<4;i++)
		pad[i] = 0;

	/* allocate memory for output array */
	status = mb_mallocd(verbose,__FILE__,__LINE__,grd.nx*grd.ny*sizeof(float),(void **)&a,error);
	if (*error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
		fprintf(outfp,"\nMBIO Error allocating output arrays.\n%s\n",
			message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, error);
		exit(status);
		}

	/* copy grid to new array and write it to GMT netCDF grd file */
	if (status == MB_SUCCESS)
		{
		/* copy grid to new array */
		for (i=0;i<grd.nx;i++)
			for (j=0;j<grd.ny;j++)
				{
				kg = i*grd.ny+j;
				ka = (grd.ny-1-j)*grd.nx+i;
				a[ka] = grid[kg];
				}

		/* write the GMT netCDF grd file */
		GMT_write_grd(outfile, &grd, a, w, e, s, n, pad, FALSE);

		/* free memory for output array */
		mb_freed(verbose,__FILE__,__LINE__,(void **) &a, error);
		}

	/* free GMT memory */
	GMT_free ((void *)GMT_io.skip_if_NaN);
	GMT_free ((void *)GMT_io.in_col_type);
	GMT_free ((void *)GMT_io.out_col_type);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(outfp,"dbg2  Return values:\n");
		fprintf(outfp,"dbg2       error:      %d\n",*error);
		fprintf(outfp,"dbg2  Return status:\n");
		fprintf(outfp,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
/*
 * function mbgrid_weight calculates the integrated weight over a bin
 * given the footprint of a sounding
 */
int mbgrid_weight(int verbose, double foot_a, double foot_b,
		    double pcx, double pcy, double dx, double dy,
		    double *px, double *py,
		    double *weight, int *use, int *error)
{
	char	*function_name = "mbgrid_weight";
	int	status = MB_SUCCESS;
	double	fa, fb;
	double	xe, ye, ang, ratio;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(outfp,"dbg2  Input arguments:\n");
		fprintf(outfp,"dbg2       verbose:    %d\n",verbose);
		fprintf(outfp,"dbg2       foot_a:     %f\n",foot_a);
		fprintf(outfp,"dbg2       foot_b:     %f\n",foot_b);
		fprintf(outfp,"dbg2       pcx:        %f\n",pcx);
		fprintf(outfp,"dbg2       pcy:        %f\n",pcy);
		fprintf(outfp,"dbg2       dx:         %f\n",dx);
		fprintf(outfp,"dbg2       dy:         %f\n",dy);
		fprintf(outfp,"dbg2       p1 x:       %f\n",px[0]);
		fprintf(outfp,"dbg2       p1 y:       %f\n",py[0]);
		fprintf(outfp,"dbg2       p2 x:       %f\n",px[1]);
		fprintf(outfp,"dbg2       p2 y:       %f\n",py[1]);
		fprintf(outfp,"dbg2       p3 x:       %f\n",px[2]);
		fprintf(outfp,"dbg2       p3 y:       %f\n",py[2]);
		fprintf(outfp,"dbg2       p4 x:       %f\n",px[3]);
		fprintf(outfp,"dbg2       p4 y:       %f\n",py[3]);
		}

	/* The weighting function is
		w(x, y) = (1 / (PI * a * b)) * exp(-(x**2/a**2 + y**2/b**2))
	    in the footprint coordinate system, where the x axis
	    is along the horizontal projection of the beam and the
	    y axix is perpendicular to that. The integral of the
	    weighting function over an simple rectangle defined
	    by corners (x1, y1), (x2, y1), (x1, y2), (x2, y2) is
		    x2 y2
		W = I  I { w(x, y) } dx dy
		    x1 y1

		  = 1 / 4 * ( erfc(x1/a) - erfc(x2/a)) * ( erfc(y1/a) - erfc(y2/a))
	    where erfc(u) is the complementary error function.
	    Each bin is represented as a simple integral in geographic
	    coordinates, but is rotated in the footprint coordinate system.
	    I can't figure out how to evaluate this integral over a
	    rotated rectangle,  and so I am crudely and incorrectly
	    approximating the integrated weight value by evaluating it over
	    the same sized rectangle centered at the same location.
	    Maybe someday I'll figure out how to do it correctly.
	    DWC 11/18/99 */

	/* get integrated weight */
	fa = foot_a;
	fb = foot_b;
/*	*weight = 0.25 * ( erfcc((pcx - dx) / fa) - erfcc((pcx + dx) / fa))
			* ( erfcc((pcy - dy) / fb) - erfcc((pcy + dy) / fb));*/
	*weight = 0.25 * ( mbgrid_erf((pcx + dx) / fa) - mbgrid_erf((pcx - dx) / fa))
			* ( mbgrid_erf((pcy + dy) / fb) - mbgrid_erf((pcy - dy) / fb));

	/* use if weight large or any ratio <= 1 */
	if (*weight > 0.05)
	    {
	    *use = MBGRID_USE_YES;
	    }
	/* check ratio of each corner footprint 1/e distance */
	else
	    {
	    *use = MBGRID_USE_NO;
	    for (i=0;i<4;i++)
		{
		ang = RTD * atan2(py[i], px[i]);
		xe = foot_a * cos(DTR * ang);
		ye = foot_b * sin(DTR * ang);
		ratio = sqrt((px[i] * px[i] + py[i] * py[i])
				/ (xe * xe + ye * ye));
		if (ratio <= 1.0)
		    *use = MBGRID_USE_YES;
		else if (ratio <= 2.0)
		    *use = MBGRID_USE_CONDITIONAL;
		}
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(outfp,"dbg2  Return values:\n");
		fprintf(outfp,"dbg2       error:      %d\n",*error);
		fprintf(outfp,"dbg2       weight:     %f\n",*weight);
		fprintf(outfp,"dbg2       use:        %d\n",*use);
		fprintf(outfp,"dbg2  Return status:\n");
		fprintf(outfp,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
/* approximate complementary error function from numerical recipies */
double erfcc(double x)
{
	double t,z,ans;

	z=fabs(x);
	t=1.0/(1.0+0.5*z);
	ans=t*exp(-z*z-1.26551223+t*(1.00002368+t*(0.37409196+t*(0.09678418+
		t*(-0.18628806+t*(0.27886807+t*(-1.13520398+t*(1.48851587+
		t*(-0.82215223+t*0.17087277)))))))));
/* fprintf(outfp, "x:%f ans:%f\n", x, ans); */
	return  x >= 0.0 ? ans : 2.0-ans;
}
/*--------------------------------------------------------------------*/
/* approximate error function altered from numerical recipies */
double mbgrid_erf(double x)
{
	double t, z, erfc_d, erf_d;

	z=fabs(x);
	t=1.0/(1.0+0.5*z);
	erfc_d=t*exp(-z*z-1.26551223+t*(1.00002368+t*(0.37409196+t*(0.09678418+
		t*(-0.18628806+t*(0.27886807+t*(-1.13520398+t*(1.48851587+
		t*(-0.82215223+t*0.17087277)))))))));
	erfc_d =  x >= 0.0 ? erfc_d : 2.0-erfc_d;
	erf_d = 1.0 - erfc_d;
	return  erf_d;
}
/*--------------------------------------------------------------------*/
