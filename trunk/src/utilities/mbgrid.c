/*--------------------------------------------------------------------
 *    The MB-system:	mbgrid.c	5/2/94
 *    $Id: mbgrid.c,v 5.12 2002-10-04 21:22:02 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 1995, 2000, 2002 by
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
 * $Log: not supported by cvs2svn $
 * Revision 5.11  2002/10/02 23:56:06  caress
 * Release 5.0.beta24
 *
 * Revision 5.10  2002/09/25 20:12:30  caress
 * Have it use fbt files again for footprint gridding algorithm.
 *
 * Revision 5.9  2002/09/20 22:30:45  caress
 * Made interpolation only fill in data gaps.
 *
 * Revision 5.8  2002/09/19 00:28:12  caress
 * Release 5.0.beta23
 *
 * Revision 5.7  2002/08/02 01:00:25  caress
 * Release 5.0.beta22
 *
 * Revision 5.6  2002/07/25 19:07:17  caress
 * Release 5.0.beta21
 *
 * Revision 5.5  2002/04/06 02:53:45  caress
 * Release 5.0.beta16
 *
 * Revision 5.4  2001/07/20 00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.3  2001/06/29 22:50:23  caress
 * Atlas Hydrosweep DS2 raw data and SURF data formats.
 *
 * Revision 5.2  2001/06/03  07:07:34  caress
 * Release 5.0.beta01.
 *
 * Revision 5.1  2001/03/22 21:15:49  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.50  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.49  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.48  2000/09/11  20:10:02  caress
 * Linked to new datalist parsing functions. Now supports recursive datalists
 * and comments in datalists.
 *
 * Revision 4.47  2000/06/20  21:00:19  caress
 * Moved execution of mbm_grdplot to after deallocation of array memory.
 *
 * Revision 4.46  1999/12/29  00:35:11  caress
 * Release 4.6.8
 *
 * Revision 4.45  1999/10/05  22:04:18  caress
 * Improved the facility for outputting ArcView grids.
 *
 * Revision 4.44  1999/09/24  23:11:07  caress
 * Altered grid interval parameter handling
 *
 * Revision 4.43  1999/09/14  21:28:50  caress
 * Fixed memory allocation errors relating to median filter gridding.
 *
 * Revision 4.42  1999/08/08  04:17:40  caress
 * Unknown changes.
 *
 * Revision 4.41  1999/04/16  01:29:39  caress
 * Version 4.6 final release?
 *
 * Revision 4.40  1999/02/04  23:55:08  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.39  1999/01/01  23:34:40  caress
 * MB-System version 4.6beta6
 *
 * Revision 4.38  1998/12/17  22:50:20  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.37  1998/10/07  19:33:56  caress
 * Removed ddmmss_to_degree function as it is included in gmt_init.c
 *
 * Revision 4.36  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.35  1997/09/15  19:11:06  caress
 * Real Version 4.5
 *
 * Revision 4.34  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.34  1997/04/17  15:14:38  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.33  1997/02/19  17:58:15  caress
 * mbgrid will now ignore datalist entries beginning with '#'.
 *
 * Revision 4.32  1997/02/18  20:39:56  caress
 * Fixed bugs where error value was not passed to functions as a pointer.
 *
 * Revision 4.31  1996/09/05  13:07:47  caress
 * Added feature that checks ".inf" files for lon lat bounds.
 *
 * Revision 4.30  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.29  1996/04/15  19:34:32  caress
 * Now mbgrid does not attempt to spline interpolate an empty grid.
 *
 * Revision 4.28  1996/01/26  21:25:58  caress
 * Version 4.3 distribution
 *
 * Revision 4.26  1995/11/28  21:03:36  caress
 * Fixed scaling for meters to feet.
 *
 * Revision 4.25  1995/11/22  22:21:36  caress
 * Now handles bathymetry in feet with -Q option.
 *
 * Revision 4.24  1995/08/17  15:04:52  caress
 * Revision for release 4.3.
 *
 * Revision 4.23  1995/08/09  13:27:57  caress
 * Adapted to GMT version 3.
 *
 * Revision 4.22  1995/05/17  21:51:20  caress
 * Stopped checking status of write_grd, as it seems nonsensical.
 *
 * Revision 4.21  1995/05/12  17:15:38  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.20  1995/04/25  19:09:03  caress
 * Now use C version of zgrid for thin plate spline interpolation.
 *
 * Revision 4.19  1995/03/22  19:52:56  caress
 * Fixed some ANSI C compliance details.
 *
 * Revision 4.18  1995/03/15  14:16:25  caress
 * Fixed trivia.
 *
 * Revision 4.17  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.16  1995/03/02  13:16:36  caress
 * Fixed bugs related to fatal error messages.
 *
 * Revision 4.15  1995/03/01  12:46:10  caress
 * Fixed typo in outputing shellscript.
 *
 * Revision 4.14  1995/02/22  21:53:14  caress
 * Added capability to ignore overlapping parts of swaths.
 *
 * Revision 4.13  1995/01/23  14:17:13  caress
 * Fixed output shellscripts to use postscript viewer specified
 * in the .mbio_defaults file.
 *
 * Revision 4.12  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.11  1994/06/21  22:53:08  caress
 * Changes to support PCs running Lynx OS.
 *
 * Revision 4.10  1994/06/17  01:23:38  caress
 * Fixed bug where null pointers in
 *    double **data;
 * were passed to free(). This was not a problem on Suns,
 * but caused core dumps on SGI's.
 *
 * Revision 4.9  1994/06/13  14:52:52  caress
 * Added capability to do median filter gridding.
 *
 * Revision 4.8  1994/06/05  22:30:01  caress
 * Added option to set grid spacing and revised info output.
 *
 * Revision 4.7  1994/06/04  02:02:01  caress
 * Fixed several bugs and made some stylistic changes to
 * the output.  Changed the data input bounds to be much
 * larger than the working grid bounds.
 *
 * Revision 4.6  1994/06/01  21:56:22  caress
 * Added ability to output topography (positive upwards) grids.
 *
 * Revision 4.5  1994/05/05  20:26:28  caress
 * Major revision. Now uses spline interpolation written in C
 * derived from GMT program surface rather than old Fortran
 * subroutine zgrid.  Also now does data processing at read
 * time, thus requiring less memory.
 *
 * Revision 4.4  1994/05/02  03:00:21  caress
 * Set output stream to stderr.
 *
 * Revision 4.3  1994/04/22  21:39:29  caress
 * Added initialization of array sgrid using memset as
 * suggested by David Brock of ASA.
 *
 * Revision 4.2  1994/04/12  18:53:44  caress
 * Added #ifdef IRIX statements for compatibility with
 * IRIX operating system. The following includes must be
 * added when compiling under IRIX: <time.h> <stdlib.h>
 *
 * Revision 4.1  1994/03/12  01:44:37  caress
 * Added declarations of ctime and/or getenv for compatability
 * with SGI compilers.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:45:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.4  1993/08/31  19:58:21  caress
 * Added -N option to allow NaN flagging of grid cells with
 * no data.  Flagging with -99999.9 is still the default.
 *
 * Revision 3.3  1993/05/16  20:21:21  caress
 * Changed the plot labeling a little bit more.
 *
 * Revision 3.2  1993/05/16  20:08:47  caress
 * Fixed bug where cfile variable was overwritten, causing the
 * output shellscript to not be executable.
 * Set clipping flag value to 99999.9 so that default gmt
 * will use white for no-data areas instead of black.
 * Changed labeling of standard deviation plots.
 *
 * Revision 3.1  1993/05/16  07:04:09  caress
 * Replaced use of NaN values as clipping flags with -99999.9 value.
 * Replaced old color table with Haxby color table.
 * Added ability to plot data distribution and standard deviation grids
 * with netcdf grd output.
 *
 * Revision 3.0  1993/05/04  22:38:46  dale
 * Inital version.
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Includes for System 5 type operating system */
#if defined (IRIX) || defined (LYNX)
#include <stdlib.h>
#endif

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"

/* GMT include files */
#include "gmt.h"

/* gridding algorithms */
#define	MBGRID_WEIGHTED_MEAN	1
#define	MBGRID_MEDIAN_FILTER	2
#define	MBGRID_MINIMUM_FILTER	3
#define	MBGRID_MAXIMUM_FILTER	4
#define	MBGRID_WEIGHTED_FOOTPRINT   5

/* grid format definitions */
#define	MBGRID_ASCII	1
#define	MBGRID_OLDGRD	2
#define	MBGRID_CDFGRD	3
#define	MBGRID_ARCASCII	4

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

/* approximate complementary error function */
double erfcc();
double mbgrid_erf();

/* program identifiers */
static char rcs_id[] = "$Id: mbgrid.c,v 5.12 2002-10-04 21:22:02 caress Exp $";
static char program_name[] = "mbgrid";
static char help_message[] =  "mbgrid is an utility used to grid bathymetry, amplitude, or \nsidescan data contained in a set of swath sonar data files.  \nThis program uses one of four algorithms (gaussian weighted mean, \nmedian filter, minimum filter, maximum filter) to grid regions \ncovered swaths and then fills in gaps between \nthe swaths (to the degree specified by the user) using a minimum\ncurvature algorithm.";
static char usage_message[] = "mbgrid -Ifilelist -Oroot \
-Rwest/east/south/north [-Adatatype\n\
          -Bborder  -Cclip -Dxdim/ydim -Edx/dy/units[!] -F\n\
          -Ggridkind -H -Jprojection -Llonflip -M -N -Ppings -Sspeed\n\
          -Ttension -Utime -V -Wscale -Xextend]";
/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	extern char *optarg;
	extern int optkind;
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
	char	file[128];
	int	file_in_bounds;
	void	*mbio_ptr = NULL;
	struct mb_io_struct *mb_io_ptr = NULL;

	/* mbgrid control variables */
	char	filelist[128];
	char	fileroot[128];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	xdim = 0;
	int	ydim = 0;
	int	spacing_priority = MB_NO;
	int	set_spacing = MB_NO;
	double	dx_set = 0.0;
	double	dy_set = 0.0;
	double	dx = 0.0;
	double	dy = 0.0;
	char	units[128];
	int	clip = 0;
	int	grid_mode = MBGRID_WEIGHTED_MEAN;
	int	datatype = MBGRID_DATA_BATHYMETRY;
	int	gridkind = MBGRID_CDFGRD;
	int	more = MB_NO;
	int	use_NaN = MB_NO;
	double	clipvalue = NO_DATA_FLAG;
	float	outclipvalue = NO_DATA_FLAG;
	double	scale = 1.0;
	double	border = 0.0;
	double	extend = 0.0;
	double	tension = 1e10;
	int	check_time = MB_NO;
	int	first_in_stays = MB_YES;
	double	timediff = 300.0;
	char	ifile[128];
	char	ofile[128];
	char	plot_cmd[256];
	int	plot_status;

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
	char	comment[256];

	/* lon,lat,value triples variables */
	double	tlon;
	double	tlat;
	double	tvalue;

	/* grid variables */
	double	gbnd[4], wbnd[4], obnd[4];
	double	xlon, ylat, xx, yy;
	double	factor, weight, topofactor;
	int	gxdim, gydim, offx, offy, xtradim;
	double	*grid = NULL;
	double	*norm = NULL;
	double	*sigma = NULL;
	double	*firsttime = NULL;
	float	*sdata = NULL;
	float	*output = NULL;
	float	*sgrid = NULL;
	int	*num = NULL;
	int	*cnt = NULL;
	double	sxmin, symin;
	float	xmin, ymin, ddx, ddy, zflag, cay;
	float	*work1 = NULL;
	float	*work2 = NULL;
	float	*work3 = NULL;
	double	**data;
	double	*value = NULL;
	int	ndata, ndatafile;
	int	time_ok;
	double	zmin, zmax, zclip;
	int	nmax;
	double	smin, smax;
	int	nbinset, nbinzero, nbinspline;
	int	bathy_in_feet = MB_NO;

	/* projected grid parameters */
	int	use_projection = MB_NO;
	int	projection_pars_f = MB_NO;
	double	reference_lon, reference_lat;
	int	utm_zone = 1;
	char	projection_type;
	char	projection_pars[128];
	char	projection_id[128];
	int	proj_status;
	void	*pjptr;
	double	p_lon_1, p_lon_2;
	double	p_lat_1, p_lat_2;
	double	deglontokm, deglattokm;
	double	mtodeglon, mtodeglat;

	/* output char strings */
	char	xlabel[MB_PATH_MAXLINE];
	char	ylabel[MB_PATH_MAXLINE];
	char	zlabel[MB_PATH_MAXLINE];
	char	title[MB_PATH_MAXLINE];
	char	nlabel[MB_PATH_MAXLINE];
	char	sdlabel[MB_PATH_MAXLINE];

	/* output stream for basic stuff (stdout if verbose <= 1,
		stderr if verbose > 1) */
	FILE	*outfp;

	/* variables needed to handle Not-a-Number values */
	float	zero = 0.0;
	float	NaN;

	/* other variables */
	FILE	*dfp;
	int	i, j, k, ii, jj, iii, jjj, kkk, n;
	int	i1, i2, j1, j2;
	double	r;
	int	dmask[9];
	int	kgrid, kout, kint, ib, ix, iy;
	int	ix1, ix2, iy1, iy2;
	int	nscan;
	
	double	foot_dx, foot_dy, foot_dxn, foot_dyn;
	double	foot_lateral, foot_range, foot_theta;
	double	foot_dtheta, foot_dphi;
	double	foot_hwidth, foot_hlength;
	int	foot_wix, foot_wiy, foot_lix, foot_liy, foot_dix, foot_diy;
	double	xx0, yy0, bdx, bdy, xx1, xx2, yy1, yy2;
	double	prx[5], pry[5];
	int	use_weight;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input and output */
	strcpy (filelist, "datalist.mb-1");

	/* initialize some values */
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
#ifndef GMT3_0
	GMT_make_fnan (GMT_f_NaN);
	GMT_make_dnan (GMT_d_NaN);
	GMT_grd_in_nan_value = GMT_grd_out_nan_value = GMT_d_NaN;
#endif

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:F:f:G:g:HhI:i:J:j:L:l:MmNnO:o:P:p:QqR:r:S:s:T:t:U:u:VvW:w:X:x:")) != -1)
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
			flag++;
			break;
		case 'C':
		case 'c':
			sscanf (optarg,"%d", &clip);
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%d/%d", &xdim, &ydim);
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
			sscanf (optarg,"%d", &gridkind);
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
			mb_get_bounds(optarg, gbnd);
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

	/* set output stream to stderr */
	outfp = stderr;

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
		fprintf(outfp,"dbg2       verbose:          %d\n",verbose);
		fprintf(outfp,"dbg2       help:             %d\n",help);
		fprintf(outfp,"dbg2       pings:            %d\n",pings);
		fprintf(outfp,"dbg2       lonflip:          %d\n",lonflip);
		fprintf(outfp,"dbg2       btime_i[0]:       %d\n",btime_i[0]);
		fprintf(outfp,"dbg2       btime_i[1]:       %d\n",btime_i[1]);
		fprintf(outfp,"dbg2       btime_i[2]:       %d\n",btime_i[2]);
		fprintf(outfp,"dbg2       btime_i[3]:       %d\n",btime_i[3]);
		fprintf(outfp,"dbg2       btime_i[4]:       %d\n",btime_i[4]);
		fprintf(outfp,"dbg2       btime_i[5]:       %d\n",btime_i[5]);
		fprintf(outfp,"dbg2       btime_i[6]:       %d\n",btime_i[6]);
		fprintf(outfp,"dbg2       etime_i[0]:       %d\n",etime_i[0]);
		fprintf(outfp,"dbg2       etime_i[1]:       %d\n",etime_i[1]);
		fprintf(outfp,"dbg2       etime_i[2]:       %d\n",etime_i[2]);
		fprintf(outfp,"dbg2       etime_i[3]:       %d\n",etime_i[3]);
		fprintf(outfp,"dbg2       etime_i[4]:       %d\n",etime_i[4]);
		fprintf(outfp,"dbg2       etime_i[5]:       %d\n",etime_i[5]);
		fprintf(outfp,"dbg2       etime_i[6]:       %d\n",etime_i[6]);
		fprintf(outfp,"dbg2       speedmin:         %f\n",speedmin);
		fprintf(outfp,"dbg2       timegap:          %f\n",timegap);
		fprintf(outfp,"dbg2       file list:        %s\n",ifile);
		fprintf(outfp,"dbg2       output file root: %s\n",fileroot);
		fprintf(outfp,"dbg2       grid x dimension: %d\n",xdim);
		fprintf(outfp,"dbg2       grid y dimension: %d\n",ydim);
		fprintf(outfp,"dbg2       grid x spacing:   %f\n",dx);
		fprintf(outfp,"dbg2       grid y spacing:   %f\n",dy);
		fprintf(outfp,"dbg2       grid bounds[0]:   %f\n",gbnd[0]);
		fprintf(outfp,"dbg2       grid bounds[1]:   %f\n",gbnd[1]);
		fprintf(outfp,"dbg2       grid bounds[2]:   %f\n",gbnd[2]);
		fprintf(outfp,"dbg2       grid bounds[3]:   %f\n",gbnd[3]);
		fprintf(outfp,"dbg2       clip:             %d\n",clip);
		fprintf(outfp,"dbg2       more:             %d\n",more);
		fprintf(outfp,"dbg2       use_NaN:          %d\n",use_NaN);
		fprintf(outfp,"dbg2       grid_mode:        %d\n",grid_mode);
		fprintf(outfp,"dbg2       data type:        %d\n",datatype);
		fprintf(outfp,"dbg2       grid format:      %d\n",gridkind);
		fprintf(outfp,"dbg2       scale:            %f\n",scale);
		fprintf(outfp,"dbg2       timediff:         %f\n",timediff);
		fprintf(outfp,"dbg2       border:           %f\n",border);
		fprintf(outfp,"dbg2       extend:           %f\n",extend);
		fprintf(outfp,"dbg2       tension:          %f\n",tension);
		fprintf(outfp,"dbg2       bathy_in_feet:    %d\n",bathy_in_feet);
		fprintf(outfp,"dbg2       projection_pars:  %s\n",projection_pars);
		fprintf(outfp,"dbg2       proj flag 1:      %d\n",projection_pars_f);
		fprintf(outfp,"dbg2       projection_id:    %s\n",projection_id);
		fprintf(outfp,"dbg2       utm_zone:         %d\n",utm_zone);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(outfp,"\n%s\n",help_message);
		fprintf(outfp,"\nusage: %s\n", usage_message);
		exit(error);
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
	if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT
		&& (datatype != MBGRID_DATA_TOPOGRAPHY
		    && datatype != MBGRID_DATA_BATHYMETRY))
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
#ifdef GMT3_0
		NaN = zero/zero;
#else
		GMT_make_fnan(NaN);
#endif
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
			obnd[0] = xlon;
			obnd[1] = xlon;
			obnd[2] = ylat;
			obnd[3] = ylat;

			/* second point */
			xx = gbnd[1];
			yy = gbnd[2];
			mb_proj_inverse(verbose, pjptr, xx, yy,
					&xlon, &ylat, &error);
			obnd[0] = MIN(obnd[0], xlon);
			obnd[1] = MAX(obnd[1], xlon);
			obnd[2] = MIN(obnd[2], ylat);
			obnd[3] = MAX(obnd[3], ylat);

			/* third point */
			xx = gbnd[0];
			yy = gbnd[3];
			mb_proj_inverse(verbose, pjptr, xx, yy,
					&xlon, &ylat, &error);
			obnd[0] = MIN(obnd[0], xlon);
			obnd[1] = MAX(obnd[1], xlon);
			obnd[2] = MIN(obnd[2], ylat);
			obnd[3] = MAX(obnd[3], ylat);

			/* fourth point */
			xx = gbnd[1];
			yy = gbnd[3];
			mb_proj_inverse(verbose, pjptr, xx, yy,
					&xlon, &ylat, &error);
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

fprintf(stderr," Projected coordinates on: proj_status:%d  projection:%s\n",
proj_status, projection_id);
fprintf(stderr," Lon Lat Bounds: %f %f %f %f\n",
obnd[0], obnd[1], obnd[2], obnd[3]);
fprintf(stderr," XY Bounds: %f %f %f %f\n",
gbnd[0], gbnd[1], gbnd[2], gbnd[3]);
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
		if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT)
			fprintf(outfp,"Footprint 1/e distance: %f times footprint\n",
				scale);
		if (check_time == MB_YES && first_in_stays == MB_NO)
			fprintf(outfp,"Swath overlap handling:       Last data used\n");
		if (check_time == MB_YES && first_in_stays == MB_YES)
			fprintf(outfp,"Swath overlap handling:       First data used\n");
		if (check_time == MB_YES)
			fprintf(outfp,"Swath overlap time threshold: %f minutes\n", 
				timediff/60.);
		if (! clip) 
			fprintf(outfp,"Spline interpolation not applied\n");
		if (clip) 
			{
			fprintf(outfp,"Spline interpolation applied with clipping dimension: %d\n",clip);
			fprintf(outfp,"Spline tension (range 0.0 to infinity): %f\n",tension);
			}
		if (gridkind == MBGRID_ASCII)
			fprintf(outfp,"Grid format %d:  ascii table\n",gridkind);
		else if (gridkind == MBGRID_CDFGRD)
			fprintf(outfp,"Grid format %d:  GMT version 2 grd (netCDF)\n",gridkind);
		else if (gridkind == MBGRID_OLDGRD)
			fprintf(outfp,"Grid format %d:  GMT version 1 grd (binary)\n",gridkind);
		else if (gridkind == MBGRID_ARCASCII)
			fprintf(outfp,"Grid format %d:  Arc/Info ascii table\n",gridkind);
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

	/* allocate memory for arrays */
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&grid,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&sigma,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&firsttime,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(int),&cnt,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(int),&num,&error);
	status = mb_malloc(verbose,xdim*ydim*sizeof(float),&output,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/***** do footprint gridding *****/
	if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT)
	{

	/* allocate memory for additional arrays */
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&norm,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize arrays */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
			grid[kgrid] = 0.0;
			norm[kgrid] = 0.0;
			sigma[kgrid] = 0.0;
			firsttime[kgrid] = 0.0;
			num[kgrid] = 0;
			cnt[kgrid] = 0;
			}

	/* read in data */
	ndata = 0;
	if (status = mb_datalist_open(verbose,&datalist,
					filelist,look_processed,&error) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	while ((status = mb_datalist_read(verbose,datalist,
			file,&format,&file_weight,&error))
			== MB_SUCCESS)
		{
		ndatafile = 0;
		
		/* if format > 0 then input is swath sonar file */
		if (format > 0 && file[0] != '#')
		{
		/* check for mbinfo file - get file bounds if possible */
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
			mb_get_fbt(verbose, file, &format, &error);
			}
		
		    /* call mb_read_init() */
		    if ((status = mb_read_init(
			verbose,file,format,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&mbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
			fprintf(outfp,"\nMultibeam File <%s> not initialized for reading\n",file);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
			
		    /* get mb_io_ptr */
		    mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

		    /* allocate memory for reading data arrays */
		    status = mb_malloc(verbose,beams_bath*sizeof(char),
				    &beamflag,&error);
		    status = mb_malloc(verbose,beams_bath*sizeof(double),
				    &bath,&error);
		    status = mb_malloc(verbose,beams_bath*sizeof(double),
				    &bathlon,&error);
		    status = mb_malloc(verbose,beams_bath*sizeof(double),
				    &bathlat,&error);
		    status = mb_malloc(verbose,beams_amp*sizeof(double),
				    &amp,&error);
		    status = mb_malloc(verbose,pixels_ss*sizeof(double),
				    &ss,&error);
		    status = mb_malloc(verbose,pixels_ss*sizeof(double),
				    &sslon,&error);
		    status = mb_malloc(verbose,pixels_ss*sizeof(double),
				    &sslat,&error);

		    /* if error initializing memory then quit */
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
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
				fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",program_name);
				fprintf(stderr,"dbg2       kind:           %d\n",kind);
				fprintf(stderr,"dbg2       beams_bath:     %d\n",beams_bath);
				fprintf(stderr,"dbg2       beams_amp:      %d\n",beams_amp);
				fprintf(stderr,"dbg2       pixels_ss:      %d\n",pixels_ss);
				fprintf(stderr,"dbg2       error:          %d\n",error);
				fprintf(stderr,"dbg2       status:         %d\n",status);
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
			      ix = (bathlon[ib] - wbnd[0] - 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] - 0.5*dy)/dy;
/*fprintf(stderr, "\nib:%d ix:%d iy:%d   bath: lon:%f lat:%f bath:%f   nav: lon:%f lat:%f\n", 
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
				/* calculate footprint - this is a kluge assuming
				   sonar at surface - also assumes lon lat grid 
				   - to be generalized in later version 
				   DWC 11/16/99 */
				/* calculate footprint - now uses sonar altitude
				   - still assumes lon lat grid 
				   - to be generalized in later version 
				   DWC 1/29/2001 */
				foot_dx = (bathlon[ib] - navlon) / mtodeglon;
				foot_dy = (bathlat[ib] - navlat) / mtodeglat;
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
				foot_theta = RTD * atan2(foot_lateral, bath[ib]);
				foot_dtheta = 0.5 * mb_io_ptr->beamwidth_xtrack;
				foot_dphi = 0.5 * mb_io_ptr->beamwidth_ltrack;
				if (foot_dtheta <= 0.0)
				    foot_dtheta = 1.0;
				if (foot_dphi <= 0.0)
				    foot_dphi = 1.0;
/*fprintf(stderr, "dx:%f dy:%f lateral:%f range:%f theta:%f\n", 
foot_dx, foot_dy, foot_lateral, foot_range, foot_theta);*/
				foot_hwidth =bath[ib] * tan(DTR * (foot_theta + foot_dtheta)) 
						    - foot_lateral;
				foot_hlength = foot_range * tan(DTR * foot_dphi);
/*fprintf(stderr, "dx:%f dy:%f mtodeglon:%f mtodeglat:%f\n", 
dx, dy, mtodeglon, mtodeglat);*/

				/* get range of bins around footprint to examine */
				foot_wix = fabs(foot_hwidth * cos(DTR * foot_theta) * mtodeglon / dx);
				foot_wiy = fabs(foot_hwidth * sin(DTR * foot_theta) * mtodeglon / dx);
				foot_lix = fabs(foot_hlength * sin(DTR * foot_theta) * mtodeglat / dy);
				foot_liy = fabs(foot_hlength * cos(DTR * foot_theta) * mtodeglat / dy);
				foot_dix = 2 * MAX(foot_wix, foot_lix);
				foot_diy = 2 * MAX(foot_wiy, foot_liy);
/*fprintf(stderr, "foot_hwidth:%f foot_hlength:%f\n", foot_hwidth, foot_hlength);
fprintf(stderr, "foot_wix:%d foot_wiy:%d  foot_lix:%d foot_liy:%d    foot_dix:%d foot_diy:%d\n", 
foot_wix, foot_wiy, foot_lix, foot_liy, foot_dix, foot_diy);*/
			        ix1 = MAX(ix - foot_dix, 0);
			        ix2 = MIN(ix + foot_dix, gxdim - 1);
			        iy1 = MAX(iy - foot_diy, 0);
			        iy2 = MIN(iy + foot_diy, gydim - 1);
/*fprintf(stderr, "ix1:%d ix2:%d iy1:%d iy2:%d\n", ix1, ix2, iy1, iy2);*/
				
				/* loop over neighborhood of bins */
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				   {
				   /* find center of bin in lon lat degrees from sounding center */
				   kgrid = ii * gydim + jj;				   
				   xx = (wbnd[0] + ii*dx + 0.5*dx - bathlon[ib]);
				   yy = (wbnd[2] + jj*dy + 0.5*dy - bathlat[ib]);
				   
				   /* get center and corners of bin in meters from sounding center */
				   xx0 = xx / mtodeglon;
				   yy0 = yy / mtodeglon;
				   bdx = 0.5 * dx/ mtodeglon;
				   bdy = 0.5 * dy/ mtodeglat;
				   xx1 = xx0 - bdx;
				   xx2 = xx0 + bdx;
				   yy1 = yy0 - bdy;
				   yy2 = yy0 + bdy;
/*fprintf(stderr, "ii:%d jj:%d ix:%d iy:%d xx:%f yy:%f\n", ii, jj, ix, iy, xx, yy);
fprintf(stderr, "p0: %f %f   p1: %f %f   p2: %f %f\n", 
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
				   mbgrid_weight(verbose, foot_hwidth, foot_hlength, scale, 
						prx[0], pry[0], bdx, bdy, 
						&prx[1], &pry[1], 
						&weight, &use_weight, &error);

				   if (use_weight != MBGRID_USE_NO)
					{
					weight *= file_weight;
					norm[kgrid] = norm[kgrid] + weight;
					grid[kgrid] = grid[kgrid] 
						+ weight*topofactor*bath[ib];
					sigma[kgrid] = sigma[kgrid] 
						+ weight*topofactor*topofactor
						*bath[ib]*bath[ib];
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
			      }
			  }
			}
		    status = mb_close(verbose,&mbio_ptr,&error);
		    mb_free(verbose,&beamflag,&error); 
		    mb_free(verbose,&bath,&error); 
		    mb_free(verbose,&bathlon,&error); 
		    mb_free(verbose,&bathlat,&error); 
		    mb_free(verbose,&amp,&error); 
		    mb_free(verbose,&ss,&error); 
		    mb_free(verbose,&sslon,&error); 
		    mb_free(verbose,&sslat,&error); 
		    status = MB_SUCCESS;
		    error = MB_ERROR_NO_ERROR;
		    }
		if (verbose >= 2) 
			fprintf(outfp,"\n");
		if (verbose > 0 || file_in_bounds == MB_YES)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,file);
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
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
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
			/*fprintf(outfp,"%d %d %d  %f %f %f   %d %d %f %f\n",
				i,j,kgrid,
				grid[kgrid], wbnd[0] + i*dx, wbnd[2] + j*dy,
				num[kgrid],cnt[kgrid],norm[kgrid],sigma[kgrid]);*/
			}

	/***** end of footprint gridding *****/
	}

	/***** do weighted mean or min/max gridding *****/
	else if (grid_mode != MBGRID_MEDIAN_FILTER)
	{

	/* allocate memory for additional arrays */
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&norm,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize arrays */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
			grid[kgrid] = 0.0;
			norm[kgrid] = 0.0;
			sigma[kgrid] = 0.0;
			firsttime[kgrid] = 0.0;
			num[kgrid] = 0;
			cnt[kgrid] = 0;
			}

	/* read in data */
	ndata = 0;
	if (status = mb_datalist_open(verbose,&datalist,
					filelist,look_processed,&error) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	while ((status = mb_datalist_read(verbose,datalist,
			file,&format,&file_weight,&error))
			== MB_SUCCESS)
		{
		ndatafile = 0;
		
		/* if format > 0 then input is swath sonar file */
		if (format > 0 && file[0] != '#')
		{
		/* check for mbinfo file - get file bounds if possible */
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
			mb_get_fbt(verbose, file, &format, &error);
			}
		
		    /* call mb_read_init() */
		    if ((status = mb_read_init(
			verbose,file,format,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&mbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
			fprintf(outfp,"\nMultibeam File <%s> not initialized for reading\n",file);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		    /* allocate memory for reading data arrays */
		    status = mb_malloc(verbose,beams_bath*sizeof(char),
				    &beamflag,&error);
		    status = mb_malloc(verbose,beams_bath*sizeof(double),
				    &bath,&error);
		    status = mb_malloc(verbose,beams_bath*sizeof(double),
				    &bathlon,&error);
		    status = mb_malloc(verbose,beams_bath*sizeof(double),
				    &bathlat,&error);
		    status = mb_malloc(verbose,beams_amp*sizeof(double),
				    &amp,&error);
		    status = mb_malloc(verbose,pixels_ss*sizeof(double),
				    &ss,&error);
		    status = mb_malloc(verbose,pixels_ss*sizeof(double),
				    &sslon,&error);
		    status = mb_malloc(verbose,pixels_ss*sizeof(double),
				    &sslat,&error);

		    /* if error initializing memory then quit */
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
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
				fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",program_name);
				fprintf(stderr,"dbg2       kind:           %d\n",kind);
				fprintf(stderr,"dbg2       beams_bath:     %d\n",beams_bath);
				fprintf(stderr,"dbg2       beams_amp:      %d\n",beams_amp);
				fprintf(stderr,"dbg2       pixels_ss:      %d\n",pixels_ss);
				fprintf(stderr,"dbg2       error:          %d\n",error);
				fprintf(stderr,"dbg2       status:         %d\n",status);
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
			      ix = (bathlon[ib] - wbnd[0] - 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] - 0.5*dy)/dy;

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
			      ix = (bathlon[ib] - wbnd[0] - 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] - 0.5*dy)/dy;

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
			      if (ss[ib] > 0.0)
				mb_proj_forward(verbose, pjptr, 
						sslon[ib], sslat[ib],
						&sslon[ib], &sslat[ib],
						&error);
			    }

			  /* deal with data */
			  for (ib=0;ib<pixels_ss;ib++) 
			    if (ss[ib] > 0.0)
			      {
			      /* get position in grid */
			      ix = (sslon[ib] - wbnd[0] - 0.5*dx)/dx;
			      iy = (sslat[ib] - wbnd[2] - 0.5*dy)/dy;

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
		    mb_free(verbose,&beamflag,&error); 
		    mb_free(verbose,&bath,&error); 
		    mb_free(verbose,&bathlon,&error); 
		    mb_free(verbose,&bathlat,&error); 
		    mb_free(verbose,&amp,&error); 
		    mb_free(verbose,&ss,&error); 
		    mb_free(verbose,&sslon,&error); 
		    mb_free(verbose,&sslat,&error); 
		    status = MB_SUCCESS;
		    error = MB_ERROR_NO_ERROR;
		    }
		if (verbose >= 2) 
			fprintf(outfp,"\n");
		if (verbose > 0 || file_in_bounds == MB_YES)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,file);
		} /* end if (format > 0) */

		/* if format == 0 then input is lon,lat,values triples file */
		else if (format == 0 && file[0] != '#')
		{
		/* open data file */
		if ((dfp = fopen(file,"r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(outfp,"\nUnable to open lon,lat,value triples data file: %s\n",
				file);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* loop over reading */
		while (fscanf(dfp,"%lf %lf %lf",&tlon,&tlat,&tvalue) != EOF)
			{
			  /* reproject data positions if necessary */
			  if (use_projection == MB_YES)
				mb_proj_forward(verbose, pjptr, 
						tlon, tlat,
						&tlon, &tlat,
						&error);

			  /* get position in grid */
			  ix = (tlon - wbnd[0] - 0.5*dx)/dx;
			  iy = (tlat - wbnd[2] - 0.5*dy)/dy;

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
		fclose(dfp);
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		if (verbose >= 2) 
			fprintf(outfp,"\n");
		if (verbose > 0)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,file);
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
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
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
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double *),&data,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize arrays */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
			grid[kgrid] = 0.0;
			sigma[kgrid] = 0.0;
			firsttime[kgrid] = 0.0;
			cnt[kgrid] = 0;
			num[kgrid] = 0;
			data[kgrid] = NULL;
			}

	/* read in data */
	ndata = 0;
	if (status = mb_datalist_open(verbose,&datalist,
					filelist,look_processed,&error) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	while ((status = mb_datalist_read(verbose,datalist,
			file,&format,&file_weight,&error))
			== MB_SUCCESS)
		{
		ndatafile = 0;

		/* if format > 0 then input is swath sonar file */
		if (format > 0 && file[0] != '#')
		{
		/* check for mbinfo file - get file bounds if possible */
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
			mb_get_fbt(verbose, file, &format, &error);
			}
		
		    /* call mb_read_init() */
		    if ((status = mb_read_init(
			verbose,file,format,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&mbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
			fprintf(outfp,"\nMultibeam File <%s> not initialized for reading\n",file);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		    /* allocate memory for reading data arrays */
		    status = mb_malloc(verbose,beams_bath*sizeof(char),
				    &beamflag,&error);
		    status = mb_malloc(verbose,beams_bath*sizeof(double),
				    &bath,&error);
		    status = mb_malloc(verbose,beams_bath*sizeof(double),
				    &bathlon,&error);
		    status = mb_malloc(verbose,beams_bath*sizeof(double),
				    &bathlat,&error);
		    status = mb_malloc(verbose,beams_amp*sizeof(double),
				    &amp,&error);
		    status = mb_malloc(verbose,pixels_ss*sizeof(double),
				    &ss,&error);
		    status = mb_malloc(verbose,pixels_ss*sizeof(double),
				    &sslon,&error);
		    status = mb_malloc(verbose,pixels_ss*sizeof(double),
				    &sslat,&error);

		    /* if error initializing memory then quit */
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
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
				fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",program_name);
				fprintf(stderr,"dbg2       kind:           %d\n",kind);
				fprintf(stderr,"dbg2       beams_bath:     %d\n",beams_bath);
				fprintf(stderr,"dbg2       beams_amp:      %d\n",beams_amp);
				fprintf(stderr,"dbg2       pixels_ss:      %d\n",pixels_ss);
				fprintf(stderr,"dbg2       error:          %d\n",error);
				fprintf(stderr,"dbg2       status:         %d\n",status);
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
			      ix = (bathlon[ib] - wbnd[0] - 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] - 0.5*dy)/dy;
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
			      ix = (bathlon[ib] - wbnd[0] - 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] - 0.5*dy)/dy;
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
			      if (ss[ib] > 0.0)
				mb_proj_forward(verbose, pjptr, 
						sslon[ib], sslat[ib],
						&sslon[ib], &sslat[ib],
						&error);
			    }

			  /* deal with data */
			  for (ib=0;ib<pixels_ss;ib++) 
			    if (ss[ib] > 0.0)
			      {
			      ix = (sslon[ib] - wbnd[0] - 0.5*dx)/dx;
			      iy = (sslat[ib] - wbnd[2] - 0.5*dy)/dy;
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
		    mb_free(verbose,&beamflag,&error); 
		    mb_free(verbose,&bath,&error); 
		    mb_free(verbose,&bathlon,&error); 
		    mb_free(verbose,&bathlat,&error); 
		    mb_free(verbose,&amp,&error); 
		    mb_free(verbose,&ss,&error); 
		    mb_free(verbose,&sslon,&error); 
		    mb_free(verbose,&sslat,&error); 
		    status = MB_SUCCESS;
		    error = MB_ERROR_NO_ERROR;
		    }
		if (verbose >= 2) 
			fprintf(outfp,"\n");
		if (verbose > 0 || file_in_bounds == MB_YES)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,file);
		} /* end if (format > 0) */

		/* if format == 0 then input is lon,lat,values triples file */
		else if (format == 0 && file[0] != '#')
		{
		/* open data file */
		if ((dfp = fopen(file,"r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(outfp,"\nUnable to open lon,lat,value triples data file: %s\n",
				file);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* loop over reading */
		while (fscanf(dfp,"%lf %lf %lf",&tlon,&tlat,&tvalue) != EOF)
			{
			  /* reproject data positions if necessary */
			  if (use_projection == MB_YES)
				mb_proj_forward(verbose, pjptr, 
						tlon, tlat,
						&tlon, &tlat,
						&error);

			  /* get position in grid */
			  ix = (tlon - wbnd[0] - 0.5*dx)/dx;
			  iy = (tlat - wbnd[2] - 0.5*dy)/dy;
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
		fclose(dfp);
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		if (verbose >= 2) 
			fprintf(outfp,"\n");
		if (verbose > 0)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,file);
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
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
			if (cnt[kgrid] > 0)
				{
				value = data[kgrid];
				qsort((char *)value,cnt[kgrid],sizeof(double),
					mb_double_compare);
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
			kgrid = i*gydim + j;
			if (cnt[kgrid] > 0)
				free(data[kgrid]);
			}

	/***** end of median filter gridding *****/
	}

	/* if clip set do smooth interpolation */
	if (clip > 0 && nbinset > 0)
		{
		/* set up data vector */
		ndata = 0;
		if (border > 0.0)
			ndata = 2*gxdim + 2*gydim - 2;
		for (i=0;i<gxdim;i++)
			for (j=0;j<gydim;j++)
				{
				kgrid = i*gydim + j;
				if (grid[kgrid] < clipvalue) ndata++;
				}

		/* allocate and initialize sgrid */
		status = mb_malloc(verbose,3*ndata*sizeof(float),&sdata,&error);
		if (status == MB_SUCCESS)
			status = mb_malloc(verbose,gxdim*gydim*sizeof(float),&sgrid,&error);
		if (status == MB_SUCCESS)
			status = mb_malloc(verbose,ndata*sizeof(float),&work1,&error);
		if (status == MB_SUCCESS)
			status = mb_malloc(verbose,ndata*sizeof(int),&work2,&error);
		if (status == MB_SUCCESS)
			status = mb_malloc(verbose,(gxdim+gydim)*sizeof(int),&work3,&error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
			fprintf(outfp,"\nMBIO Error allocating interpolation work arrays:\n%s\n",
				message);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		memset((char *)sgrid,0,gxdim*gydim*sizeof(float));

		/* get points from grid */
		sxmin = gbnd[0] - offx*dx;
		symin = gbnd[2] - offy*dy;
		ndata = 0;
		for (i=0;i<gxdim;i++)
			for (j=0;j<gydim;j++)
				{
				kgrid = i*gydim + j;
				if (grid[kgrid] < clipvalue)
					{
					sdata[ndata++] = sxmin + dx*i;
					sdata[ndata++] = symin + dy*j;
					sdata[ndata++] = grid[kgrid];
					}
				}
		/* if desired set border */
		if (border > 0.0)
			{
			for (i=0;i<gxdim;i++)
				{
				j = 0;
				kgrid = i*gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sdata[ndata++] = sxmin + dx*i;
					sdata[ndata++] = symin + dy*j;
					sdata[ndata++] = border;
					}
				j = gydim - 1;
				kgrid = i*gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sdata[ndata++] = sxmin + dx*i;
					sdata[ndata++] = symin + dy*j;
					sdata[ndata++] = border;
					}
				}
			for (j=1;j<gydim-1;j++)
				{
				i = 0;
				kgrid = i*gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sdata[ndata++] = sxmin + dx*i;
					sdata[ndata++] = symin + dy*j;
					sdata[ndata++] = border;
					}
				i = gxdim - 1;
				kgrid = i*gydim + j;
				if (grid[kgrid] >= clipvalue)
					{
					sdata[ndata++] = sxmin + dx*i;
					sdata[ndata++] = symin + dy*j;
					sdata[ndata++] = border;
					}
				}
			}
		ndata = ndata/3;

		/* do the interpolation */
		if (verbose > 0)
		    fprintf(outfp,"\nDoing spline interpolation with %d data points...\n",ndata);
		cay = tension;
		xmin = sxmin;
		ymin = symin;
		ddx = dx;
		ddy = dy;
		mb_zgrid(sgrid,&gxdim,&gydim,&xmin,&ymin,
			&ddx,&ddy,sdata,&ndata,
			work1,work2,work3,&cay,&clip);

		/* translate the interpolation into the grid array 
		    - interpolate only to fill a data gap */
		zflag = 5.0e34;
		for (i=0;i<gxdim;i++)
		    for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
			kint = i + j*gxdim;
			num[kgrid] = MB_NO;
			if (grid[kgrid] >= clipvalue 
			    && sgrid[kint] < zflag)
			    {
			    /* initialize direction mask 
				and bounds of search */
			    for (ii=0;ii<9;ii++)
				dmask[ii] = MB_NO;
			    i1 = MAX(0, i - clip);
			    i2 = MIN(gxdim - 1, i + clip);
			    j1 = MAX(0, j - clip);
			    j2 = MIN(gydim - 1, j + clip);
				    
			    /* loop over data within clip region */
			    for (ii=i1;ii<=i2;ii++)
				for (jj=j1;jj<=j2;jj++)
				    {
				    if (grid[ii*gydim+jj] < clipvalue)
					{
					r = sqrt((double)((ii-i)*(ii-i) + (jj-j)*(jj-j)));
					iii = rint((ii - i)/r) + 1;
					jjj = rint((jj - j)/r) + 1;
					kkk = iii * 3 + jjj;
					dmask[kkk] = MB_YES;
					}
				    }
				    
			    if ((dmask[0] && dmask[8])
				|| (dmask[3] && dmask[5])
				|| (dmask[6] && dmask[2])
				|| (dmask[1] && dmask[7]))
				num[kgrid] = MB_YES;
			    }
			}
		for (i=0;i<gxdim;i++)
		    for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
			kint = i + j*gxdim;
			if (num[kgrid] == MB_YES)
				{
				grid[kgrid] = sgrid[kint];
				nbinspline++;
				}
			}
		mb_free(verbose,&sdata,&error);
		mb_free(verbose,&sgrid,&error);
		mb_free(verbose,&work1,&error);
		mb_free(verbose,&work2,&error);
		mb_free(verbose,&work3,&error);
		}

	/* get min max of data */
	zclip = clipvalue;
	zmin = zclip;
	zmax = zclip;
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;;
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
			kgrid = i*gydim + j;;
			if (cnt[kgrid] > nmax)
				nmax = cnt[kgrid];
			}

	/* get min max of standard deviation */
	smin = 0.0;
	smax = 0.0;
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;;
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
	nbinzero = gxdim*gydim - nbinset - nbinspline;
	fprintf(outfp,"\nTotal number of bins:            %d\n",gxdim*gydim);
	fprintf(outfp,"Bins set using data:             %d\n",nbinset);
	fprintf(outfp,"Bins set using interpolation:    %d\n",nbinspline);
	fprintf(outfp,"Bins not set:                    %d\n",nbinzero);
	fprintf(outfp,"Maximum number of data in a bin: %d\n",nmax);
	fprintf(outfp,"Minimum value: %10.2f   Maximum value: %10.2f\n",
		zmin,zmax);
	fprintf(outfp,"Minimum sigma: %10.5f   Maximum sigma: %10.5f\n",
		smin,smax);

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
				&& grid[kgrid] == clipvalue)
				output[kout] = outclipvalue;
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
	if (status != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nError writing output file: %s\n%s\n",
			ofile,message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
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
		if (status != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nError writing output file: %s\n%s\n",
				ofile,message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
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
		if (status != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nError writing output file: %s\n%s\n",
				ofile,message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		}

	/* deallocate arrays */
	mb_free(verbose,&grid,&error); 
	mb_free(verbose,&norm,&error); 
	mb_free(verbose,&num,&error); 
	mb_free(verbose,&cnt,&error); 
	mb_free(verbose,&sigma,&error); 
	mb_free(verbose,&firsttime,&error); 
	mb_free(verbose,&output,&error); 

	/* run mbm_grdplot */
	if (gridkind == MBGRID_CDFGRD)
		{
		/* execute mbm_grdplot */
		strcpy(ofile,fileroot);
		strcat(ofile,".grd");
		if (datatype == MBGRID_DATA_BATHYMETRY)
			{
			sprintf(plot_cmd, "mbm_grdplot -I%s -G1 -C -D -V -L\"File %s - %s:%s\"", 
				ofile, ofile, title, zlabel);
			}
		else if (datatype == MBGRID_DATA_TOPOGRAPHY)
			{
			sprintf(plot_cmd, "mbm_grdplot -I%s -G1 -C -V -L\"File %s - %s:%s\"", 
				ofile, ofile, title, zlabel);
			}
		else if (datatype == MBGRID_DATA_AMPLITUDE)
			{
			sprintf(plot_cmd, "mbm_grdplot -I%s -G1 -W1/4 -S -D -V -L\"File %s - %s:%s\"", 
				ofile, ofile, title, zlabel);
			}
		else
			{
			sprintf(plot_cmd, "mbm_grdplot -I%s -G1 -W1/4 -S -D -V -L\"File %s - %s:%s\"", 
				ofile, ofile, title, zlabel);
			}
		if (verbose)
			{
			fprintf(stderr, "\nexecuting mbm_grdplot...\n%s\n", 
				plot_cmd);
			}
		plot_status = system(plot_cmd);
		if (plot_status == -1)
			{
			fprintf(stderr, "\nError executing mbm_grdplot on output file %s\n", ofile);
			}
		}
	if (more == MB_YES
		&& gridkind == MBGRID_CDFGRD)
		{
		/* execute mbm_grdplot */
		strcpy(ofile,fileroot);
		strcat(ofile,"_num.grd");
		sprintf(plot_cmd, "mbm_grdplot -I%s -G1 -W1/2 -V -L\"File %s - %s:%s\"", 
			ofile, ofile, title, nlabel);
		if (verbose)
			{
			fprintf(stderr, "\nexecuting mbm_grdplot...\n%s\n", 
				plot_cmd);
			}
		plot_status = system(plot_cmd);
		if (plot_status == -1)
			{
			fprintf(stderr, "\nError executing mbm_grdplot on output file grd_%s\n", fileroot);
			}

		/* execute mbm_grdplot */
		strcpy(ofile,fileroot);
		strcat(ofile,"_sd.grd");
		sprintf(plot_cmd, "mbm_grdplot -I%s -G1 -W1/2 -V -L\"File %s - %s:%s\"", 
			ofile, ofile, title, sdlabel);
		if (verbose)
			{
			fprintf(stderr, "\nexecuting mbm_grdplot...\n%s\n", 
				plot_cmd);
			}
		plot_status = system(plot_cmd);
		if (plot_status == -1)
			{
			fprintf(stderr, "\nError executing mbm_grdplot on output file grd_%s\n", fileroot);
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
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
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
	char	date[25], user[128], *user_ptr, host[128];
	char	*ctime();
	char	*getenv();

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       outfile:    %s\n",outfile);
		fprintf(stderr,"dbg2       grid:       %d\n",grid);
		fprintf(stderr,"dbg2       nx:         %d\n",nx);
		fprintf(stderr,"dbg2       ny:         %d\n",ny);
		fprintf(stderr,"dbg2       xmin:       %f\n",xmin);
		fprintf(stderr,"dbg2       xmax:       %f\n",xmax);
		fprintf(stderr,"dbg2       ymin:       %f\n",ymin);
		fprintf(stderr,"dbg2       ymax:       %f\n",ymax);
		fprintf(stderr,"dbg2       dx:         %f\n",dx);
		fprintf(stderr,"dbg2       dy:         %f\n",dy);
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
		strncpy(date,"\0",25);
		strncpy(date,ctime(&right_now),24);
		if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
		if (user_ptr != NULL)
			strcpy(user,user_ptr);
		else
			strcpy(user, "unknown");
		i = gethostname(host,128);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       outfile:    %s\n",outfile);
		fprintf(stderr,"dbg2       grid:       %d\n",grid);
		fprintf(stderr,"dbg2       nx:         %d\n",nx);
		fprintf(stderr,"dbg2       ny:         %d\n",ny);
		fprintf(stderr,"dbg2       xmin:       %f\n",xmin);
		fprintf(stderr,"dbg2       xmax:       %f\n",xmax);
		fprintf(stderr,"dbg2       ymin:       %f\n",ymin);
		fprintf(stderr,"dbg2       ymax:       %f\n",ymax);
		fprintf(stderr,"dbg2       dx:         %f\n",dx);
		fprintf(stderr,"dbg2       dy:         %f\n",dy);
		fprintf(stderr,"dbg2       nodata:     %f\n",nodata);
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
		fprintf(fp, "xllcorner %.10g\n", xmin);
		fprintf(fp, "yllcorner %.10g\n", ymin);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       outfile:    %s\n",outfile);
		fprintf(stderr,"dbg2       grid:       %d\n",grid);
		fprintf(stderr,"dbg2       nx:         %d\n",nx);
		fprintf(stderr,"dbg2       ny:         %d\n",ny);
		fprintf(stderr,"dbg2       xmin:       %f\n",xmin);
		fprintf(stderr,"dbg2       xmax:       %f\n",xmax);
		fprintf(stderr,"dbg2       ymin:       %f\n",ymin);
		fprintf(stderr,"dbg2       ymax:       %f\n",ymax);
		fprintf(stderr,"dbg2       dx:         %f\n",dx);
		fprintf(stderr,"dbg2       dy:         %f\n",dy);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
	int	pad[4];
	int	complex;
	float	*a;
	time_t	right_now;
	char	date[128], user[128], *user_ptr, host[128];
	int	i, j, kg, ka;
	char	*ctime();
	char	*getenv();

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       outfile:    %s\n",outfile);
		fprintf(stderr,"dbg2       grid:       %d\n",grid);
		fprintf(stderr,"dbg2       nx:         %d\n",nx);
		fprintf(stderr,"dbg2       ny:         %d\n",ny);
		fprintf(stderr,"dbg2       xmin:       %f\n",xmin);
		fprintf(stderr,"dbg2       xmax:       %f\n",xmax);
		fprintf(stderr,"dbg2       ymin:       %f\n",ymin);
		fprintf(stderr,"dbg2       ymax:       %f\n",ymax);
		fprintf(stderr,"dbg2       dx:         %f\n",dx);
		fprintf(stderr,"dbg2       dy:         %f\n",dy);
		fprintf(stderr,"dbg2       xlab:       %s\n",xlab);
		fprintf(stderr,"dbg2       ylab:       %s\n",ylab);
		fprintf(stderr,"dbg2       zlab:       %s\n",zlab);
		fprintf(stderr,"dbg2       titl:       %s\n",titl);
		fprintf(stderr,"dbg2       argc:       %d\n",argc);
		fprintf(stderr,"dbg2       *argv:      %d\n",*argv);
		}

	/* inititialize grd header */
#ifdef GMT3_0
	grdio_init();
	grd_init (&grd, argc, argv, MB_NO);
#else
	GMT_grdio_init();
	GMT_grd_init (&grd, argc, argv, MB_NO);
#endif

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
	strncpy(date,"\0",128);
	right_now = time((time_t *)0);
	strncpy(date,ctime(&right_now),24);
	if ((user_ptr = getenv("USER")) == NULL)
		user_ptr = getenv("LOGNAME");
	if (user_ptr != NULL)
		strcpy(user,user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host,128);
	sprintf(grd.remark,"\n\tProjection: %s\n\tGrid created by %s\n\tMB-system Version %s\n\tRun by <%s> on <%s> at <%s>",
		projection,program_name,MB_VERSION,user,host,date);

	/* set extract wesn,pad and complex */
	w = 0.0;
	e = 0.0;
	s = 0.0;
	n = 0.0;
	for (i=0;i<4;i++)
		pad[i] = 0;
	complex = 0;

	/* allocate memory for output array */
	status = mb_malloc(verbose,grd.nx*grd.ny*sizeof(float),&a,error);

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
#ifdef GMT3_0
		write_grd(outfile, &grd, a, w, e, s, n, pad, complex);
#else
		GMT_write_grd(outfile, &grd, a, w, e, s, n, pad, complex);
#endif

		/* free memory for output array */
		mb_free(verbose, &a, error);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
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
/*
 * function mbgrid_weight calculates the integrated weight over a bin
 * given the footprint of a sounding
 */
int mbgrid_weight(int verbose, double foot_a, double foot_b, double scale, 
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
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       foot_a:     %f\n",foot_a);
		fprintf(stderr,"dbg2       foot_b:     %f\n",foot_b);
		fprintf(stderr,"dbg2       scale:      %f\n",scale);
		fprintf(stderr,"dbg2       pcx:        %f\n",pcx);
		fprintf(stderr,"dbg2       pcy:        %f\n",pcy);
		fprintf(stderr,"dbg2       dx:         %f\n",dx);
		fprintf(stderr,"dbg2       dy:         %f\n",dy);
		fprintf(stderr,"dbg2       p1 x:       %f\n",px[0]);
		fprintf(stderr,"dbg2       p1 y:       %f\n",py[0]);
		fprintf(stderr,"dbg2       p2 x:       %f\n",px[1]);
		fprintf(stderr,"dbg2       p2 y:       %f\n",py[1]);
		fprintf(stderr,"dbg2       p3 x:       %f\n",px[2]);
		fprintf(stderr,"dbg2       p3 y:       %f\n",py[2]);
		fprintf(stderr,"dbg2       p4 x:       %f\n",px[3]);
		fprintf(stderr,"dbg2       p4 y:       %f\n",py[3]);
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
	fa = scale * foot_a;
	fb = scale * foot_b;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       weight:     %f\n",*weight);
		fprintf(stderr,"dbg2       use:        %d\n",*use);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
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
/* fprintf(stderr, "x:%f ans:%f\n", x, ans); */
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
