/*--------------------------------------------------------------------
 *    The MB-system:	mbmosaic.c	2/10/97
 *    $Id$
 *
 *    Copyright (c) 1997-2012 by
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
 * mbmosaic is an utility used to mosaic amplitude or sidescan
 * data contained in a set of swath mapping sonar data files.
 * This program mosaics the data using a prioritization scheme
 * tied to the apparent grazing angle and look azimuth for the
 * pixels/beams. The grazing
 * angle is calculated as arctan(xtrack / depth) where the
 * acrosstrack distance xtrack is positive to starboard.
 *
 * Author:	D. W. Caress
 * Date:	February 10, 1997
 *
 * $Log: mbmosaic.c,v $
 * Revision 5.31  2009/03/02 18:54:40  caress
 * Fixed pixel size problems with mbmosaic, resurrected program mbfilter, and also updated copyright dates in several source files.
 *
 * Revision 5.30  2008/12/22 08:36:18  caress
 * Check in of 22 Dec 2008.
 *
 * Revision 5.29  2008/09/27 03:27:11  caress
 * Working towards release 5.1.1beta24
 *
 * Revision 5.28  2008/08/12 00:04:04  caress
 * Gordon Keith's addition of a weighting option.
 *
 * Revision 5.27  2008/05/24 19:40:07  caress
 * Applied a Gordon Keith fix.
 *
 * Revision 5.26  2008/01/14 18:35:49  caress
 * Improved handling of datalists.
 *
 * Revision 5.25  2007/10/08 16:48:07  caress
 * State of the code on 8 October 2007.
 *
 * Revision 5.24  2006/09/11 18:55:54  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.23  2006/08/09 22:41:27  caress
 * Fixed programs that read or write grids so that they do not use the GMT_begin() function; these programs will now work when GMT is built in the default fashion, when GMT is built in the default fashion, with "advisory file locking" enabled.
 *
 * Revision 5.22  2006/06/22 04:45:43  caress
 * Working towards 5.1.0
 *
 * Revision 5.21  2006/04/11 19:19:29  caress
 * Various fixes.
 *
 * Revision 5.20  2006/02/01 07:31:06  caress
 * Modifications suggested by Gordon Keith
 *
 * Revision 5.19  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.18  2005/11/05 01:07:54  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.17  2004/12/02 06:38:50  caress
 * Fix suggested by Gordon Keith
 *
 * Revision 5.16  2003/12/12 01:39:06  caress
 * Fixed designation of the output stream to stdout or stderr.
 *
 * Revision 5.15  2003/04/17 21:18:57  caress
 * Release 5.0.beta30
 *
 * Revision 5.14  2002/11/14 03:52:25  caress
 * Release 5.0.beta27
 *
 * Revision 5.13  2002/11/12 07:23:58  caress
 * Added mb_memory_clear() calls.
 *
 * Revision 5.12  2002/11/04 21:26:55  caress
 * Fixed memory leak using proj.
 *
 * Revision 5.11  2002/10/04 21:22:02  caress
 * Now resets lonflip to specified bounds. Release 5.0.beta24.
 *
 * Revision 5.10  2002/10/02 23:56:06  caress
 * Release 5.0.beta24
 *
 * Revision 5.9  2002/09/25 20:12:30  caress
 * Not sure what I did....
 *
 * Revision 5.8  2002/09/20 22:30:45  caress
 * Made interpolation only fill in data gaps.
 *
 * Revision 5.7  2002/09/19 00:28:12  caress
 * Release 5.0.beta23
 *
 * Revision 5.6  2002/08/02 01:00:25  caress
 * Release 5.0.beta22
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
 * Revision 4.12  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.11  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.10  2000/09/11  20:10:02  caress
 * Linked to new datalist parsing functions. Now supports recursive datalists
 * and comments in datalists.
 *
 * Revision 4.9  2000/06/20  21:00:19  caress
 * Moved execution of mbm_grdplot to after deallocation of array memory.
 *
 * Revision 4.8  1999/10/05  22:04:18  caress
 * Improved the facility for outputting ArcView grids.
 *
 * Revision 4.7  1999/09/24  23:11:07  caress
 * Altered grid interval parameter handling
 *
 * Revision 4.6  1999/08/08  04:17:40  caress
 * Unknown changes.
 *
 * Revision 4.5  1999/04/16  01:29:39  caress
 * Version 4.6 final release?
 *
 * Revision 4.4  1999/02/04  23:55:08  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.3  1998/10/07  19:33:56  caress
 * Removed ddmmss_to_degree function as it is included in gmt_init.c
 *
 * Revision 4.2  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.1  1997/09/15  19:11:06  caress
 * Real Version 4.5
 *
 * Revision 4.0  1997/04/21  17:17:47  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1997/04/17  15:14:38  caress
 * MB-System 4.5 Beta Release
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

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_info.h"
#include "../../include/mb_process.h"
#include "../../include/mb_aux.h"

/* GMT include files */
#include "gmt.h"

/* gridding algorithms */
#define	MBMOSAIC_SINGLE_BEST	1
#define	MBMOSAIC_AVERAGE	2

/* grid format definitions */
#define	MBMOSAIC_ASCII		1
#define	MBMOSAIC_OLDGRD		2
#define	MBMOSAIC_CDFGRD		3
#define	MBMOSAIC_ARCASCII	4
#define	MBMOSAIC_GMTGRD		100

/* gridded data type */
#define	MBMOSAIC_DATA_AMPLITUDE		3
#define	MBMOSAIC_DATA_SIDESCAN		4
#define MBMOSAIC_DATA_FLAT_GRAZING	5
#define MBMOSAIC_DATA_GRAZING		6
#define MBMOSAIC_DATA_SLOPE		7
#define MBMOSAIC_DATA_OTHER		8

/* prioritization mode */
#define	MBMOSAIC_PRIORITY_NONE		0
#define	MBMOSAIC_PRIORITY_ANGLE		1
#define	MBMOSAIC_PRIORITY_AZIMUTH	2
#define	MBMOSAIC_PRIORITY_BOTH		3

/* priority tables */
#define	MBMOSAIC_PRIORITYTABLE_FILE		0
#define	MBMOSAIC_PRIORITYTABLE_60DEGREESUP	1
#define	MBMOSAIC_PRIORITYTABLE_67DEGREESUP	2
#define	MBMOSAIC_PRIORITYTABLE_75DEGREESUP	3
#define	MBMOSAIC_PRIORITYTABLE_85DEGREESUP	4
#define	MBMOSAIC_PRIORITYTABLE_60DEGREESDN	5
#define	MBMOSAIC_PRIORITYTABLE_67DEGREESDN	6
#define	MBMOSAIC_PRIORITYTABLE_75DEGREESDN	7
#define	MBMOSAIC_PRIORITYTABLE_85DEGREESDN	8
int	n_priority_angle_60degreesup = 3;
double	priority_angle_60degreesup_angle[] = {-60, 0, 60};
double	priority_angle_60degreesup_priority[] = {1.0, 0.0, 1.0};
int	n_priority_angle_67degreesup = 3;
double	priority_angle_67degreesup_angle[] = {-67, 0, 67};
double	priority_angle_67degreesup_priority[] = {1.0, 0.0, 1.0};
int	n_priority_angle_75degreesup = 3;
double	priority_angle_75degreesup_angle[] = {-75, 0, 75};
double	priority_angle_75degreesup_priority[] = {1.0, 0.0, 1.0};
int	n_priority_angle_85degreesup = 3;
double	priority_angle_85degreesup_angle[] = {-85, 0, 85};
double	priority_angle_85degreesup_priority[] = {1.0, 0.0, 1.0};
int	n_priority_angle_60degreesdn = 3;
double	priority_angle_60degreesdn_angle[] = {-60, 0, 60};
double	priority_angle_60degreesdn_priority[] = {0.0, 1.0, 0.0};
int	n_priority_angle_67degreesdn = 3;
double	priority_angle_67degreesdn_angle[] = {-67, 0, 67};
double	priority_angle_67degreesdn_priority[] = {0.0, 1.0, 0.0};
int	n_priority_angle_75degreesdn = 3;
double	priority_angle_75degreesdn_angle[] = {-75, 0, 75};
double	priority_angle_75degreesdn_priority[] = {0.0, 1.0, 0.0};
int	n_priority_angle_85degreesdn = 3;
double	priority_angle_85degreesdn_angle[] = {-85, 0, 85};
double	priority_angle_85degreesdn_priority[] = {0.0, 1.0, 0.0};

#define	MB7K2SS_NUM_ANGLES			171
#define	MB7K2SS_ANGLE_MAX			85.0

/* flag for no data in grid */
#define	NO_DATA_FLAG	99999

/* interpolation mode */
#define MBMOSAIC_INTERP_NONE	0
#define MBMOSAIC_INTERP_GAP	1
#define MBMOSAIC_INTERP_NEAR	2
#define MBMOSAIC_INTERP_ALL	3

#define MBMOSAIC_FOOTPRINT_REAL		0
#define MBMOSAIC_FOOTPRINT_SPACING	1
struct	footprint
	{
	double	x[4];
	double	y[4];
	};

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
int mbmosaic_get_beamangles(
		int     verbose,
                double  sonardepth,
                int     beams_bath,
                char    *beamflag,
                double  *bath,
                double  *bathacrosstrack,
                double  *bathalongtrack,
                double  *gangles,
                int     *error);
int mbmosaic_get_beampriorities(
                int     verbose,
                int     priority_mode,
                int     n_priority_angle,
                double  *priority_angle_angle,
                double  *priority_angle_priority,
                double  priority_azimuth,
                double  priority_azimuth_factor,
                double  heading,
                int     beams_bath,
                char    *beamflag,
                double  *gangles,
                double  *priorities,
                int     *error);
int mbmosaic_get_beamslopes(
                int     verbose,
                int     beams_bath,
                char    *beamflag,
                double  *bath,
                double  *bathacrosstrack,
                double  *slopes,
                int     *error);
int mbmosaic_bath_getangletable(
                int     verbose,
                double  sonardepth,
                int     beams_bath,
                char    *beamflag,
                double  *bath,
                double  *bathacrosstrack,
                double  *bathalongtrack,
                double  angle_min,
                double  angle_max,
                int     nangle,
                double  *table_angle,
                double  *table_xtrack,
                double  *table_ltrack,
		double  *table_altitude,
                double  *table_range,
                int     *error);
int mbmosaic_flatbottom_getangletable(
                int     verbose,
                double  altitude,
                double  angle_min,
                double  angle_max,
                int     nangle,
                double  *table_angle,
                double  *table_xtrack,
                double  *table_ltrack,
		double  *table_altitude,
                double  *table_range,
                int     *error);
int mbmosaic_get_ssangles(
                int     verbose,
                int     nangle,
                double  *table_angle,
                double  *table_xtrack,
                double  *table_ltrack,
                double  *table_altitude,
                double  *table_range,
                int  pixels_ss,
                double  *ss,
                double  *ssacrosstrack,
                double  *gangles,
                int  *error);
int mbmosaic_get_sspriorities(
                int     verbose,
                int     priority_mode,
                int     n_priority_angle,
                double  *priority_angle_angle,
                double  *priority_angle_priority,
                double  priority_azimuth,
                double  priority_azimuth_factor,
                double  heading,
                int     pixels_ss,
                double  *ss,
                double  *gangles,
                double  *priorities,
                int     *error);
int mbmosaic_get_footprint(
		int	verbose,
		int	mode,
		double	beamwidth_xtrack,
		double	beamwidth_ltrack,
		double	altitude,
		double	acrosstrack,
		double	alongtrack,
		double	acrosstrack_spacing,
		struct footprint *footprint,
		int	*error);
int get_other_data(
		void	*mbio_ptr,
		int	beam,
		int	datatype,
		double	*data,
		int	*error);

/* program identifiers */
static char rcs_id[] = "$Id$";
char program_name[] = "mbmosaic";
char help_message[] =  "mbmosaic is an utility used to mosaic amplitude or \nsidescan data contained in a set of swath sonar data files.  \nThis program uses one of four algorithms (gaussian weighted mean, \nmedian filter, minimum filter, maximum filter) to grid regions \ncovered by multibeam swaths and then fills in gaps between \nthe swaths (to the degree specified by the user) using a minimum\ncurvature algorithm.";
char usage_message[] = "mbmosaic -Ifilelist -Oroot \
[-Rwest/east/south/north -Rfactor -Adatatype\n\
          -Bborder -Cclip/mode/tension -Dxdim/ydim -Edx/dy/units \n\
          -Fpriority_range -Ggridkind -H -Jprojection -Llonflip -M -N -Ppings \n\
          -Sspeed -Ttopogrid -Uazimuth/factor -V -Wscale -Xextend \n\
          -Ypriority_source -Zbathdef]";

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
        void    *store_ptr;

	/* mbmosaic control variables */
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
	int	clipmode = MBMOSAIC_INTERP_NONE;
	double	tension = 0.0;
	int	grid_mode = MBMOSAIC_SINGLE_BEST;
	int	datatype = MBMOSAIC_DATA_SIDESCAN;
	int	usefiltered = MB_NO;
	char	gridkindstring[MB_PATH_MAXLINE];
	int	gridkind = MBMOSAIC_GMTGRD;
	int	more = MB_NO;
	int	use_NaN = MB_NO;
	double	clipvalue = NO_DATA_FLAG;
	float	outclipvalue = NO_DATA_FLAG;
	double	scale = 1.0;
        double  boundsfactor = 0.0;
	double	border = 0.0;
	double	extend = 0.0;
	int	priority_mode = MBMOSAIC_PRIORITY_NONE;
	int	priority_source = MBMOSAIC_PRIORITYTABLE_FILE;
	double	priority_range = 0.0;
	double	priority_azimuth = 0.0;
	double	priority_azimuth_factor = 1.0;
	char	pfile[MB_PATH_MAXLINE];
	int	n_priority_angle = 0;
	double	*priority_angle_angle = NULL;
	double	*priority_angle_priority = NULL;
	int	weight_priorities = 0;
        int     usetopogrid = MB_NO;
	double	altitude_default = 1000.0;
	int	pstatus;
	char	path[MB_PATH_MAXLINE];
	char	ppath[MB_PATH_MAXLINE];
	char	ifile[MB_PATH_MAXLINE];
	char	ofile[MB_PATH_MAXLINE];
	char	dfile[MB_PATH_MAXLINE];
	char	plot_cmd[MB_COMMENT_MAXLINE];
	int	plot_status;
	int	use_beams = MB_NO;
	int 	use_slope = MB_NO;
	int	linear = MB_NO;
	double	value;

	/* topography parameters */
        mb_path topogridfile;
	void	*topogrid_ptr = NULL;

	/* mbio read values */
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
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*bathlon = NULL;
	double	*bathlat = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	double	*sslon = NULL;
	double	*sslat = NULL;
	char	comment[MB_COMMENT_MAXLINE];
	double	*gangles = NULL;
	double	*slopes = NULL;
	double	*priorities = NULL;
	struct	footprint *footprints = NULL;
	struct mb_info_struct mb_info;
	int	formatread;
	double	beamwidth_xtrack;
	double	beamwidth_ltrack;
        double  draft;
        double  roll;
        double  pitch;
        double  heave;

	/* grid variables */
	double	gbnd[4], wbnd[4], obnd[4];
	int	gbndset = MB_NO;
	double	xlon, ylat, xx, yy;
	double	gaussian_factor;
	int	gxdim, gydim, offx, offy;
	double	*grid = NULL;
	double	*norm = NULL;
	double	*maxpriority = NULL;
	int	*cnt = NULL;
	int	*num = NULL;
	double	*sigma = NULL;
	float	*sdata = NULL;
	float	*output = NULL;
	float	*sgrid = NULL;
	double	sxmin, symin;
	float	xmin, ymin, ddx, ddy, zflag, cay;
	void	*work1 = NULL;
	void	*work2 = NULL;
	void	*work3 = NULL;
	int	ndata, ndatafile;
	double	zmin, zmax, zclip;
	int	nmax;
	double	smin, smax;
	int	nbinset, nbinzero, nbinspline;

	/* bottom layout parameters */
	int	nangle = MB7K2SS_NUM_ANGLES;
	double	angle_min = -MB7K2SS_ANGLE_MAX;
	double	angle_max = MB7K2SS_ANGLE_MAX;
	double	table_angle[MB7K2SS_NUM_ANGLES];
	double	table_xtrack[MB7K2SS_NUM_ANGLES];
	double	table_ltrack[MB7K2SS_NUM_ANGLES];
	double	table_altitude[MB7K2SS_NUM_ANGLES];
	double	table_range[MB7K2SS_NUM_ANGLES];
        int     table_status = MB_SUCCESS;
        int     table_error = MB_ERROR_NO_ERROR;

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
	double	headingx, headingy;

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
	float	NaN;

	/* other variables */
	FILE	*dfp, *fp;
	char	buffer[MB_PATH_MAXLINE], *result;
	double	norm_weight;
	double	xsmin, xsmax;
        double  xx1, yy1;
	int	ismin, ismax;
	int	footprint_mode;
	int	inside;
	double	acrosstrackspacing;
        double  slope;
	int	i, j, ii, jj, iii, jjj, kkk, n;
	int	i1, i2, j1, j2;
        int     ir;
	double	r;
	int	dmask[9];
	int	kgrid, kout, kint, ib;
	int	ixx[4], iyy[4];
	int	ix1, ix2, iy1, iy2;

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

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:F:f:G:g:HhI:i:J:j:L:l:MmNnO:o:P:p:R:r:S:s:T:t:U:u:VvW:w:X:x:Y:y:Z:z:")) != -1)
	  switch (c)
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%d", &datatype);
                        if (optarg[1] == 'f' || optarg[1] == 'F')
				usefiltered = MB_YES;
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%lf", &border);
			flag++;
			break;
		case 'C':
		case 'c':
			n = sscanf (optarg,"%d/%d/%lf", &clip, &clipmode,&tension);
			if (n < 1)
				clipmode = MBMOSAIC_INTERP_NONE;
			else if (n == 1 && clip > 0)
				clipmode = MBMOSAIC_INTERP_GAP;
			else if (n == 1)
				clipmode = MBMOSAIC_INTERP_NONE;
			else if (clip > 0 && clipmode < 0)
				clipmode = MBMOSAIC_INTERP_GAP;
			else if (clipmode >= 3)
				clipmode = MBMOSAIC_INTERP_ALL;
                        if (n < 3)
                                {
                                tension = 0.0;
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
			n = sscanf (optarg,"%lf/%lf/%s",
				    &dx_set, &dy_set, units);
			if (n > 1)
				set_spacing = MB_YES;
			if (n < 3)
				strcpy(units, "meters");
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%lf/%d", &priority_range, &weight_priorities, &linear);
			grid_mode = MBMOSAIC_AVERAGE;
			flag++;
			break;
		case 'G':
		case 'g':
			if (optarg[0] == '=')
				{
				gridkind = MBMOSAIC_GMTGRD;
				strcpy(gridkindstring, optarg);
				}
			else
				{
				sscanf (optarg,"%d", &gridkind);
				if (gridkind == MBMOSAIC_CDFGRD)
					{
					gridkind = MBMOSAIC_GMTGRD;
					gridkindstring[0] = '\0';
					}
				else if (gridkind > MBMOSAIC_GMTGRD)
					{
					sprintf(gridkindstring, "=%d", (gridkind - 100));
					gridkind = MBMOSAIC_GMTGRD;
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
			sscanf (optarg,"%s", topogridfile);
			usetopogrid = MB_YES;
			flag++;
fprintf(stderr,"dbg2      usetopogrid:          %d\n",usetopogrid);
fprintf(stderr,"dbg2      topogridfile:         %s\n",topogridfile);
			break;
		case 'U':
		case 'u':
			sscanf (optarg,"%lf/%lf",
			    &priority_azimuth, &priority_azimuth_factor);
			if (priority_mode == MBMOSAIC_PRIORITY_ANGLE)
			    priority_mode = MBMOSAIC_PRIORITY_BOTH;
			else
			    priority_mode = MBMOSAIC_PRIORITY_AZIMUTH;
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
		case 'Y':
		case 'y':
			sscanf (optarg,"%d", &priority_source);
			if (priority_source == MBMOSAIC_PRIORITYTABLE_60DEGREESUP)
				{
				n_priority_angle = n_priority_angle_60degreesup;
				priority_angle_angle = priority_angle_60degreesup_angle;
				priority_angle_priority = priority_angle_60degreesup_priority;
				}
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_67DEGREESUP)
				{
				n_priority_angle = n_priority_angle_67degreesup;
				priority_angle_angle = priority_angle_67degreesup_angle;
				priority_angle_priority = priority_angle_67degreesup_priority;
				}
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_75DEGREESUP)
				{
				n_priority_angle = n_priority_angle_75degreesup;
				priority_angle_angle = priority_angle_75degreesup_angle;
				priority_angle_priority = priority_angle_75degreesup_priority;
				}
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_85DEGREESUP)
				{
				n_priority_angle = n_priority_angle_85degreesup;
				priority_angle_angle = priority_angle_85degreesup_angle;
				priority_angle_priority = priority_angle_85degreesup_priority;
				}
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_60DEGREESDN)
				{
				n_priority_angle = n_priority_angle_60degreesdn;
				priority_angle_angle = priority_angle_60degreesdn_angle;
				priority_angle_priority = priority_angle_60degreesdn_priority;
				}
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_67DEGREESDN)
				{
				n_priority_angle = n_priority_angle_67degreesdn;
				priority_angle_angle = priority_angle_67degreesdn_angle;
				priority_angle_priority = priority_angle_67degreesdn_priority;
				}
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_75DEGREESDN)
				{
				n_priority_angle = n_priority_angle_75degreesdn;
				priority_angle_angle = priority_angle_75degreesdn_angle;
				priority_angle_priority = priority_angle_75degreesdn_priority;
				}
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_85DEGREESDN)
				{
				n_priority_angle = n_priority_angle_85degreesdn;
				priority_angle_angle = priority_angle_85degreesdn_angle;
				priority_angle_priority = priority_angle_85degreesdn_priority;
				}
			else
				{
				sscanf (optarg,"%s", pfile);
				}
			if (priority_mode == MBMOSAIC_PRIORITY_AZIMUTH)
			    priority_mode = MBMOSAIC_PRIORITY_BOTH;
			else
			    priority_mode = MBMOSAIC_PRIORITY_ANGLE;
			break;
		case 'Z':
		case 'z':
			sscanf (optarg,"%lf", &altitude_default);
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
		fprintf(outfp,"dbg2       file list:            %s\n",ifile);
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
		fprintf(outfp,"dbg2       more:                 %d\n",more);
		fprintf(outfp,"dbg2       use_NaN:              %d\n",use_NaN);
		fprintf(outfp,"dbg2       data type:            %d\n",datatype);
		fprintf(outfp,"dbg2       usefiltered:          %d\n",usefiltered);
		fprintf(outfp,"dbg2       grid format:          %d\n",gridkind);
		if (gridkind == MBMOSAIC_GMTGRD)
		fprintf(outfp,"dbg2       gmt grid format id:   %s\n",gridkindstring);
		fprintf(outfp,"dbg2       scale:                %f\n",scale);
		fprintf(outfp,"dbg2       border:               %f\n",border);
		fprintf(outfp,"dbg2       extend:               %f\n",extend);
		fprintf(outfp,"dbg2       tension:              %f\n",tension);
		fprintf(outfp,"dbg2       grid_mode:            %d\n",grid_mode);
		fprintf(outfp,"dbg2       priority_mode:        %d\n",priority_mode);
		fprintf(outfp,"dbg2       priority_range:       %f\n",priority_range);
		fprintf(outfp,"dbg2       weight_priorities:    %d\n",weight_priorities);
		fprintf(outfp,"dbg2       priority_source:      %d\n",priority_source);
		fprintf(outfp,"dbg2       pfile:                %s\n",pfile);
		fprintf(outfp,"dbg2       priority_azimuth:     %f\n",priority_azimuth);
		fprintf(outfp,"dbg2       priority_azimuth_fac: %f\n",priority_azimuth_factor);
		fprintf(outfp,"dbg2       altitude_default:     %f\n",altitude_default);
		fprintf(outfp,"dbg2       projection_pars:      %s\n",projection_pars);
		fprintf(outfp,"dbg2       proj flag 1:          %d\n",projection_pars_f);
		fprintf(outfp,"dbg2       projection_id:        %s\n",projection_id);
		fprintf(outfp,"dbg2       utm_zone:             %d\n",utm_zone);
		fprintf(stderr,"dbg2      usetopogrid:          %d\n",usetopogrid);
		fprintf(stderr,"dbg2      topogridfile:         %s\n",topogridfile);
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

	/* use bathymetry/amplitude beams for types other than sidescan */
	if (datatype == MBMOSAIC_DATA_SIDESCAN)
	  	use_beams = MB_NO;
	else
	  	use_beams = MB_YES;

	/* use bathymetry slope for slope and slope corrected grazing angle */
	if (datatype == MBMOSAIC_DATA_GRAZING
		|| datatype == MBMOSAIC_DATA_SLOPE)
		use_slope = MB_YES;

	/* more option not available with single best algorithm */
	if (more == MB_YES
		&& grid_mode == MBMOSAIC_SINGLE_BEST)
		more = MB_NO;

	/* NaN cannot be used for ASCII grids */
	if (use_NaN == MB_YES
		&& (gridkind == MBMOSAIC_ASCII
		    || gridkind == MBMOSAIC_ARCASCII))
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
			xdim = (gbnd[1] - gbnd[0])/(mtodeglon * 0.3048 * dx_set) + 1;
			if (dy_set <= 0.0)
				dy_set = mtodeglon * dx_set / mtodeglat;
			ydim = (gbnd[3] - gbnd[2])/(mtodeglat * 0.3048 * dy_set) + 1;
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
	gaussian_factor = 4.0/(scale*scale*dx*dy);
	offx = 0;
	offy = 0;
	if (extend > 0.0)
		{
		offx = (int) (extend*xdim);
		offy = (int) (extend*ydim);
		}
	gxdim = xdim + 2*offx;
	gydim = ydim + 2*offy;
	wbnd[0] = gbnd[0] - offx*dx;
	wbnd[1] = gbnd[1] + offx*dx;
	wbnd[2] = gbnd[2] - offy*dy;
	wbnd[3] = gbnd[3] + offy*dy;

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
	if ((clipmode == MBMOSAIC_INTERP_GAP
		|| clipmode == MBMOSAIC_INTERP_NEAR)
		&& clip > xdim && clip > ydim)
		clipmode = MBMOSAIC_INTERP_ALL;
        if (clipmode == MBMOSAIC_INTERP_ALL)
                clip = MAX(xdim, ydim);

	/* if specified get static angle priorities */
	if (priority_source == MBMOSAIC_PRIORITYTABLE_FILE
		&& (priority_mode == MBMOSAIC_PRIORITY_ANGLE
			|| priority_mode == MBMOSAIC_PRIORITY_BOTH))
		{
		/* count priorities */
		if ((fp = fopen(pfile, "r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Angle Weights File <%s> for reading\n",pfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		n_priority_angle = 0;
		while ((result = fgets(buffer,MB_PATH_MAXLINE,fp)) == buffer)
			{
			if (buffer[0] != '#')
				{
				n_priority_angle++;
				}
			}
		fclose(fp);

		/* allocate memory */
		if (error == MB_ERROR_NO_ERROR)
			status = mb_mallocd(verbose,__FILE__,__LINE__,n_priority_angle*sizeof(double),
				(void **)&priority_angle_angle,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_mallocd(verbose,__FILE__,__LINE__,n_priority_angle*sizeof(double),
				(void **)&priority_angle_priority,&error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}

		/* read in angle priorities */
		if ((fp = fopen(pfile, "r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Angle Weights File <%s> for reading\n",pfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		n_priority_angle = 0;
		while ((result = fgets(buffer,MB_PATH_MAXLINE,fp)) == buffer)
			{
			if (buffer[0] != '#')
				{
				sscanf(buffer,"%lf %lf",
					&priority_angle_angle[n_priority_angle],
					&priority_angle_priority[n_priority_angle]);
				n_priority_angle++;
				}
			}
		fclose(fp);
		}

	/* read topography grid if 3D bottom correction specified */
	if (usetopogrid == MB_YES)
		{
		status = mb_topogrid_init(verbose, topogridfile, &lonflip, &topogrid_ptr, &error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error loading topography grid: %s\n%s\n",topogridfile,message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		}

	/* output info */
	if (verbose >= 0)
		{
		fprintf(outfp,"\nMBMOSAIC Parameters:\n");
		fprintf(outfp,"List of input files: %s\n",filelist);
		fprintf(outfp,"Output fileroot:     %s\n",fileroot);
		fprintf(outfp,"Input Data Type:     ");
		if (datatype == MBMOSAIC_DATA_AMPLITUDE && usefiltered == MB_NO)
			fprintf(outfp,"Amplitude (unfiltered)\n");
		else if (datatype == MBMOSAIC_DATA_AMPLITUDE && usefiltered == MB_YES)
			fprintf(outfp,"Amplitude (filtered)\n");
		else if (datatype == MBMOSAIC_DATA_SIDESCAN && usefiltered == MB_NO)
			fprintf(outfp,"Sidescan (unfiltered)\n");
		else if (datatype == MBMOSAIC_DATA_SIDESCAN && usefiltered == MB_YES)
			fprintf(outfp,"Sidescan (filtered)\n");
		else if (datatype == MBMOSAIC_DATA_FLAT_GRAZING)
			fprintf(outfp,"Flat bottom grazing angle\n");
		else if (datatype == MBMOSAIC_DATA_GRAZING)
			fprintf(outfp,"Grazing angle\n");
		else if (datatype == MBMOSAIC_DATA_SLOPE)
			fprintf(outfp,"Bottom slope\n");
		else if (datatype >= MBMOSAIC_DATA_OTHER)
			fprintf(outfp,"Programmed value %d\n", datatype);
		else
			fprintf(outfp,"Unknown?\n");
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
		fprintf(outfp,"Mosaicing algorithm:  \n");
		if (grid_mode == MBMOSAIC_SINGLE_BEST)
			fprintf(outfp,"  Single highest weighted pixel\n");
		else if (grid_mode == MBMOSAIC_AVERAGE)
			{
			fprintf(outfp,"  Average of highest weighted pixels\n");
			fprintf(outfp,"  Pixel weighting range: %f\n", priority_range);
			}
		if (priority_mode == MBMOSAIC_PRIORITY_NONE)
			fprintf(outfp, "  All pixels weighted evenly\n");
		if (priority_mode == MBMOSAIC_PRIORITY_ANGLE
			|| priority_mode == MBMOSAIC_PRIORITY_BOTH)
			{
			fprintf(outfp, "  Pixels prioritized by flat bottom grazing angle\n");
                        if (usetopogrid == MB_YES)
				fprintf(outfp, "  Pixel depths calculated from topography grid: %s\n", topogridfile);
                        else
				fprintf(outfp, "  Pixel depths calculated from topoography in the swath file\n");
			if (priority_source == MBMOSAIC_PRIORITYTABLE_FILE)
				fprintf(outfp, "  Pixel prioritization file: %s\n", pfile);
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_60DEGREESUP)
				fprintf(outfp, "  Pixel prioritization model: default 120 degree swath increasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_67DEGREESUP)
				fprintf(outfp, "  Pixel prioritization model: default 134 degree swath increasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_75DEGREESUP)
				fprintf(outfp, "  Pixel prioritization model: default 150 degree swath increasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_85DEGREESUP)
				fprintf(outfp, "  Pixel prioritization model: default 170 degree swath increasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_60DEGREESDN)
				fprintf(outfp, "  Pixel prioritization model: default 120 degree swath decreasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_67DEGREESDN)
				fprintf(outfp, "  Pixel prioritization model: default 134 degree swath decreasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_75DEGREESDN)
				fprintf(outfp, "  Pixel prioritization model: default 150 degree swath decreasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_85DEGREESDN)
				fprintf(outfp, "  Pixel prioritization model: default 170 degree swath decreasing out\n");
			fprintf(outfp, "  Grazing angle priorities:\n");
			for (i=0;i<n_priority_angle;i++)
				{
				fprintf(outfp,"    %3d  %10.3f  %10.3f\n",
				i, priority_angle_angle[i],priority_angle_priority[i]);
				}
			}
		if (priority_mode == MBMOSAIC_PRIORITY_AZIMUTH
			|| priority_mode == MBMOSAIC_PRIORITY_BOTH)
			{
			fprintf(outfp, "  Pixels weighted by look azimuth\n");
			fprintf(outfp, "  Preferred look azimuth: %f\n", priority_azimuth);
			fprintf(outfp, "  Look azimuth factor:    %f\n", priority_azimuth_factor);
			}
		fprintf(outfp,"  Gaussian filter 1/e length: %f grid intervals\n",
				scale);
		if (clipmode == MBMOSAIC_INTERP_NONE)
			fprintf(outfp,"Spline interpolation not applied\n");
		else if (clipmode == MBMOSAIC_INTERP_GAP)
			{
			fprintf(outfp,"Spline interpolation applied to fill data gaps\n");
			fprintf(outfp,"Spline interpolation clipping dimension: %d\n",clip);
			fprintf(outfp,"Spline tension (range 0.0 to infinity): %f\n",tension);
			}
		else if (clipmode == MBMOSAIC_INTERP_NEAR)
			{
			fprintf(outfp,"Spline interpolation applied near data\n");
			fprintf(outfp,"Spline interpolation clipping dimension: %d\n",clip);
			fprintf(outfp,"Spline tension (range 0.0 to infinity): %f\n",tension);
			}
		else if (clipmode == MBMOSAIC_INTERP_ALL)
			{
			fprintf(outfp,"Spline interpolation applied to fill entire grid\n");
			fprintf(outfp,"Spline tension (range 0.0 to infinity): %f\n",tension);
			}
		if (gridkind == MBMOSAIC_ASCII)
			fprintf(outfp,"Grid format %d:  ascii table\n",gridkind);
		else if (gridkind == MBMOSAIC_CDFGRD)
			fprintf(outfp,"Grid format %d:  GMT version 2 grd (netCDF)\n",gridkind);
		else if (gridkind == MBMOSAIC_OLDGRD)
			fprintf(outfp,"Grid format %d:  GMT version 1 grd (binary)\n",gridkind);
		else if (gridkind == MBMOSAIC_ARCASCII)
			fprintf(outfp,"Grid format %d:  Arc/Info ascii table\n",gridkind);
		else if (gridkind == MBMOSAIC_GMTGRD)
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

	/* allocate memory for arrays */
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(double),(void **)&grid,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(double),(void **)&norm,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(double),(void **)&maxpriority,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(int),(void **)&cnt,&error);
	if (clip != 0)
	    status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(int),(void **)&num,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,gxdim*gydim*sizeof(double),(void **)&sigma,&error);
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

	/* initialize arrays */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
			grid[kgrid] = 0.0;
			norm[kgrid] = 0.0;
			cnt[kgrid] = 0;
			sigma[kgrid] = 0.0;
			maxpriority[kgrid] = 0.0;
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

	/***** do first pass gridding *****/
	if (grid_mode == MBMOSAIC_SINGLE_BEST
	    || priority_mode != MBMOSAIC_PRIORITY_NONE)
	{

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

		/* if format > 0 then input is multibeam file */
		if (format > 0)
		{
		/* apply pstatus */
		if (pstatus == MB_PROCESSED_USE)
			strcpy(file, ppath);
		else
			strcpy(file, path);

		/* check for mbinfo file - get file bounds if possible */
		status = mb_check_info(verbose, file, lonflip, bounds,
				&file_in_bounds, &error);
		if (status == MB_FAILURE)
			{
			file_in_bounds = MB_YES;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* initialize the multibeam file */
		if (file_in_bounds == MB_YES)
		    {
		    /* check for filtered amplitude or sidescan file */
		    if (usefiltered == MB_YES && datatype == MBMOSAIC_DATA_AMPLITUDE)
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
		    else if (usefiltered == MB_YES && datatype == MBMOSAIC_DATA_SIDESCAN)
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

		    /* open the file */
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
			mb_memory_clear(verbose, &error);
			exit(error);
			}

                    /* get pointers to data storage */
                    mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
                    store_ptr = mb_io_ptr->store_data;

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
							    sizeof(double), (void **)&bathacrosstrack, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathalongtrack, &error);
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
							    sizeof(double), (void **)&ssacrosstrack, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&ssalongtrack, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslat, &error);
		    if (datatype != MBMOSAIC_DATA_SIDESCAN)
		    	{
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&gangles, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&slopes, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&priorities, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(struct footprint), (void **)&footprints, &error);
			}
		    else
		    	{
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&gangles, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&priorities, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(struct footprint), (void **)&footprints, &error);
			}
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&work1, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&work2, &error);

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
                        status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
                                            time_i,&time_d,&navlon,&navlat,
                                            &speed,&heading,
                                            &distance,&altitude,&sonardepth,
                                            &beams_bath,&beams_amp,&pixels_ss,
                                            beamflag,bath,amp,bathacrosstrack,bathalongtrack,
                                            ss,ssacrosstrack,ssalongtrack,
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

			if (status == MB_SUCCESS && kind == MB_DATA_DATA)
                            {
                            status = mb_extract_nav(verbose,mbio_ptr,store_ptr,&kind,
                                                    time_i,&time_d,&navlon,&navlat,
                                                    &speed,&heading,&draft,
                                                    &roll,&pitch,&heave,&error);

                            /* get factors for lon lat calculations */
                            if (error == MB_ERROR_NO_ERROR)
                                    {
                                    mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
                                    headingx = sin(DTR*heading);
                                    headingy = cos(DTR*heading);
                                    }

                            /* get beam widths */
                            if (error == MB_ERROR_NO_ERROR)
                                    {
                                    status = mb_beamwidths(verbose, mbio_ptr, &beamwidth_xtrack, &beamwidth_ltrack, &error);
                                    }

                            /* mosaic beam based data (amplitude, grazing angle, slope) */
                            if (use_beams == MB_YES
                                    && error == MB_ERROR_NO_ERROR)
                              {
                              /* translate beam locations to lon/lat */
                              for (ib=0;ib<beams_amp;ib++)
                                {
                                if (mb_beam_ok(beamflag[ib]))
                                    {
                                    /* handle regular beams */
                                    bathlon[ib] = navlon
                                        + headingy * mtodeglon
                                            * bathacrosstrack[ib]
                                        + headingx * mtodeglon
                                            * bathalongtrack[ib];
                                    bathlat[ib] = navlat
                                        - headingx * mtodeglat
                                            * bathacrosstrack[ib]
                                        + headingy * mtodeglat
                                            * bathalongtrack[ib];

                                    /* get footprints */
                                    mbmosaic_get_footprint(verbose, MBMOSAIC_FOOTPRINT_REAL,
                                                            beamwidth_xtrack, beamwidth_ltrack,
                                                            (bath[ib] - sonardepth),
                                                            bathacrosstrack[ib], bathalongtrack[ib],
                                                            0.0, &footprints[ib], &error);
                                    for (j=0;j<4;j++)
                                            {
                                            xx = navlon
                                                + headingy * mtodeglon
                                                    * footprints[ib].x[j]
                                                + headingx * mtodeglon
                                                    * footprints[ib].y[j];
                                            yy = navlat
                                                - headingx * mtodeglat
                                                    * footprints[ib].x[j]
                                                + headingy * mtodeglat
                                                    * footprints[ib].y[j];
                                            footprints[ib].x[j] = xx;
                                            footprints[ib].y[j] = yy;
                                            }
                                    }
                                }

                              /* get beam angles */
                              mbmosaic_get_beamangles(verbose, sonardepth, beams_bath, beamflag, bath, bathacrosstrack, bathalongtrack,
                                                      gangles, &error);

                              /* get priorities */
                              mbmosaic_get_beampriorities(verbose, priority_mode,
                                    n_priority_angle, priority_angle_angle, priority_angle_priority,
                                    priority_azimuth, priority_azimuth_factor,
                                    heading, beams_bath, beamflag, gangles, priorities, &error);

                              /* get bathymetry slopes if needed */
                              if (use_slope == MB_YES)
                                    mbmosaic_get_beamslopes(verbose,beams_bath,beamflag,bath,bathacrosstrack,slopes,&error);

                              /* reproject beam positions if necessary */
                              if (use_projection == MB_YES)
                                {
                                for (ib=0;ib<beams_amp;ib++)
                                  if (mb_beam_ok(beamflag[ib]))
                                    {
                                    mb_proj_forward(verbose, pjptr,
                                                    bathlon[ib], bathlat[ib],
                                                    &bathlon[ib], &bathlat[ib],
                                                    &error);
                                    for (j=0;j<4;j++)
                                            {
                                            mb_proj_forward(verbose, pjptr,
                                                    footprints[ib].x[j], footprints[ib].y[j],
                                                    &footprints[ib].x[j], &footprints[ib].y[j],
                                                    &error);
                                            }
                                    }
                                }

                              /* deal with data */
                              for (ib=0;ib<beams_amp;ib++)
                                if (mb_beam_ok(beamflag[ib]))
                                  {
                                  /* get position in grid */
                                  for (j=0;j<4;j++)
                                     {
                                     ixx[j] = (footprints[ib].x[j] - wbnd[0] + 0.5*dx)/dx;
                                     iyy[j] = (footprints[ib].y[j] - wbnd[2] + 0.5*dy)/dy;
                                     }
                                  ix1 = ixx[0];
                                  iy1 = iyy[0];
                                  ix2 = ixx[0];
                                  iy2 = iyy[0];
                                  for (j=1;j<4;j++)
                                     {
                                     ix1 = MIN(ix1, ixx[j]);
                                     iy1 = MIN(iy1, iyy[j]);
                                     ix2 = MAX(ix2, ixx[j]);
                                     iy2 = MAX(iy2, iyy[j]);
                                     }
    /*		              dix = (int)(scale * (ix2 - ix1));
                                  diy = (int)(scale * (iy2 - iy1));
                                  ix1 = MAX(ix1 - dix, 0);
                                  ix2 = MIN(ix2 + dix, gxdim - 1);
                                  iy1 = MAX(iy1 - diy, 0);
                                  iy2 = MIN(iy2 + diy, gydim - 1);*/
                                  ix1 = MAX(ix1, 0);
                                  ix2 = MIN(ix2, gxdim - 1);
                                  iy1 = MAX(iy1, 0);
                                  iy2 = MIN(iy2, gydim - 1);

                                  /* process if in region of interest */
                                    for (ii=ix1;ii<=ix2;ii++)
                                     for (jj=iy1;jj<=iy2;jj++)
                                        {
                                        /* set grid if highest weight */
                                        kgrid = ii*gydim + jj;
                                        xx = dx * ii + wbnd[0];
                                        yy = dy * jj + wbnd[2];
                                        inside = mb_pr_point_in_quad(verbose, xx, yy,
                                                                    footprints[ib].x, footprints[ib].y,
                                                                    &error);
                                        if (inside == MB_YES
                                            && priorities[ib] > maxpriority[kgrid])
                                            {
                                            if (datatype == MBMOSAIC_DATA_AMPLITUDE)
                                                grid[kgrid] = amp[ib];
                                            else if (datatype == MBMOSAIC_DATA_FLAT_GRAZING)
                                              {
                                                if (gangles[ib] > 0)
                                                  grid[kgrid] = gangles[ib];
                                                else
                                                  grid[kgrid] = - gangles[ib];
                                              }
                                            else if (datatype == MBMOSAIC_DATA_GRAZING)
                                              {
                                                slope = slopes[ib] + gangles[ib];
                                                if (slope < 0)
                                                  slope = -slope;
                                                grid[kgrid] = slope;
                                              }
                                            else if (datatype == MBMOSAIC_DATA_SLOPE)
                                              {
                                                slope = slopes[ib];
                                                if (slope < 0)
                                                  slope = -slope;
                                                grid[kgrid] = slope;
                                              }
					    else if (datatype >= MBMOSAIC_DATA_OTHER)
					      {
					        get_other_data(mbio_ptr, ib, datatype, &grid[kgrid], &error);
					      }

                                            cnt[kgrid] = 1;
                                            maxpriority[kgrid] = priorities[ib];
                                            }
                                        }
                                  ndata++;
                                  ndatafile++;
                                  }
                              }

                           /* mosaic sidescan */
                            else if (datatype == MBMOSAIC_DATA_SIDESCAN
                                    && error == MB_ERROR_NO_ERROR)
                              {
                              /* get spacing */
                              xsmin = 0.0;
                              xsmax = 0.0;
                              ismin = pixels_ss / 2;
                              ismax = pixels_ss / 2;
                              for (ib=0;ib<pixels_ss;ib++)
                                {
                                if (ss[ib] > MB_SIDESCAN_NULL)
                                    {
                                    if (ssacrosstrack[ib] < xsmin)
                                            {
                                            xsmin = ssacrosstrack[ib];
                                            ismin = ib;
                                            }
                                    if (ssacrosstrack[ib] > xsmax)
                                            {
                                            xsmax = ssacrosstrack[ib];
                                            ismax = ib;
                                            }
                                    }
                                }
                              if (ismax > ismin)
                                    {
                                    footprint_mode = MBMOSAIC_FOOTPRINT_SPACING;
                                    acrosstrackspacing = (xsmax - xsmin) / (ismax - ismin);
                                    }
                              else
                                    {
                                    footprint_mode = MBMOSAIC_FOOTPRINT_REAL;
                                    acrosstrackspacing = 0.0;
                                    }

                              /* translate pixel locations to lon/lat */
                              for (ib=0;ib<pixels_ss;ib++)
                                {
                                if (ss[ib] > MB_SIDESCAN_NULL)
                                    {
                                    sslon[ib] = navlon
                                        + headingy * mtodeglon
                                            * ssacrosstrack[ib]
                                        + headingx * mtodeglon
                                            * ssalongtrack[ib];
                                    sslat[ib] = navlat
                                        - headingx * mtodeglat
                                            * ssacrosstrack[ib]
                                        + headingy * mtodeglat
                                            * ssalongtrack[ib];
    /*fprintf(stderr,"ib:%d ss:%f  x:%f l:%f  lon:%f lat:%f fprnt:",
    ib,ss[ib],ssacrosstrack[ib],ssalongtrack[ib],sslon[ib],sslat[ib]);*/

                                    /* get footprints */
                                    mbmosaic_get_footprint(verbose, footprint_mode,
                                                            beamwidth_xtrack, beamwidth_ltrack,
                                                            altitude,
                                                            ssacrosstrack[ib], ssalongtrack[ib],
                                                            acrosstrackspacing, &footprints[ib], &error);
                                    for (j=0;j<4;j++)
                                            {
                                            xx = navlon
                                                + headingy * mtodeglon
                                                    * footprints[ib].x[j]
                                                + headingx * mtodeglon
                                                    * footprints[ib].y[j];
                                            yy = navlat
                                                - headingx * mtodeglat
                                                    * footprints[ib].x[j]
                                                + headingy * mtodeglat
                                                    * footprints[ib].y[j];
    /*fprintf(stderr," %f %f",footprints[ib].x[j],footprints[ib].y[j]);*/
                                            footprints[ib].x[j] = xx;
                                            footprints[ib].y[j] = yy;
                                            }
    /*fprintf(stderr,"\n");*/
                                    }
                                }

                              /* get angle vs acrosstrack distance table using topographic grid */
                              table_error = MB_ERROR_NO_ERROR;
                              if (usetopogrid == MB_YES)
                                {
                                table_status = mb_topogrid_getangletable(verbose, topogrid_ptr, nangle, angle_min, angle_max,
                                                                            navlon, navlat, heading,
                                                                            altitude, sonardepth, pitch,
                                                                            table_angle, table_xtrack, table_ltrack,
                                                                            table_altitude, table_range, &table_error);
                                if (table_status == MB_FAILURE)
                                    {
                                    mb_error(verbose,table_error,&message);
                                    fprintf(outfp,"\nMBIO Error extracting topography from grid for sidescan:\n%s\n",message);
                                    fprintf(outfp,"\nNonfatal error in program <%s>\n",program_name);
                                    fprintf(outfp,"Requested angle-distance table extends beyond the bounds of the topography grid <%s>\n",topogridfile);
                                    fprintf(outfp,"used for grazing angle calculation - flat bottom calculation used in places.\n");
                                    table_status = MB_SUCCESS;
                                    table_error = MB_ERROR_NO_ERROR;
                                    }
                                }

                              /* get angle vs acrosstrack distance table using bathymetry from the swath file with sidescan */
                              else
                                {
                                table_status = mbmosaic_bath_getangletable(verbose, sonardepth, beams_bath, beamflag, bath, bathacrosstrack, bathalongtrack,
                                                                            angle_min, angle_max, nangle, table_angle, table_xtrack, table_ltrack,
                                                                            table_altitude, table_range, &table_error);
                                }

                              /* if need be, calculate angles using flat bottom layout and nadir altitude */
                              if (table_status == MB_FAILURE)
                                {
                                if (altitude <= 0.0)
                                    altitude = altitude_default;
                                table_status = mbmosaic_flatbottom_getangletable(verbose, altitude,
                                                                            angle_min, angle_max, nangle, table_angle, table_xtrack, table_ltrack,
                                                                            table_altitude, table_range, &table_error);
                                }

                              /* get angles for each pixel */
                              mbmosaic_get_ssangles(verbose, nangle, table_angle, table_xtrack, table_ltrack, table_altitude, table_range,
                                                                            pixels_ss, ss, ssacrosstrack, gangles, &error);

                              /* get priorities for each pixel */
                              mbmosaic_get_sspriorities(verbose, priority_mode,
                                    n_priority_angle, priority_angle_angle, priority_angle_priority,
                                    priority_azimuth, priority_azimuth_factor,
                                    heading, pixels_ss, ss, gangles, priorities, &error);

                              /* reproject pixel positions if necessary */
                              if (use_projection == MB_YES)
                                {
                                for (ib=0;ib<pixels_ss;ib++)
                                  if (ss[ib] > MB_SIDESCAN_NULL)
                                    {
                                    mb_proj_forward(verbose, pjptr,
                                                    sslon[ib], sslat[ib],
                                                    &sslon[ib], &sslat[ib],
                                                    &error);
                                    for (j=0;j<4;j++)
                                            {
                                            mb_proj_forward(verbose, pjptr,
                                                    footprints[ib].x[j], footprints[ib].y[j],
                                                    &footprints[ib].x[j], &footprints[ib].y[j],
                                                    &error);
                                            }
                                    }
                                }

                              /* deal with data */
                              for (ib=0;ib<pixels_ss;ib++)
                                if (ss[ib] > MB_SIDESCAN_NULL)
                                  {
                                  /* get position in grid */
                                  for (j=0;j<4;j++)
                                     {
                                     ixx[j] = (footprints[ib].x[j] - wbnd[0] + 0.5*dx)/dx;
                                     iyy[j] = (footprints[ib].y[j] - wbnd[2] + 0.5*dy)/dy;
                                     }
                                  ix1 = ixx[0];
                                  iy1 = iyy[0];
                                  ix2 = ixx[0];
                                  iy2 = iyy[0];
                                  for (j=1;j<4;j++)
                                     {
                                     ix1 = MIN(ix1, ixx[j]);
                                     iy1 = MIN(iy1, iyy[j]);
                                     ix2 = MAX(ix2, ixx[j]);
                                     iy2 = MAX(iy2, iyy[j]);
                                     }
    /*		              dix = (int)(scale * (ix2 - ix1));
                                  diy = (int)(scale * (iy2 - iy1));
                                  ix1 = MAX(ix1 - dix, 0);
                                  ix2 = MIN(ix2 + dix, gxdim - 1);
                                  iy1 = MAX(iy1 - diy, 0);
                                  iy2 = MIN(iy2 + diy, gydim - 1);*/
                                  ix1 = MAX(ix1, 0);
                                  ix2 = MIN(ix2, gxdim - 1);
                                  iy1 = MAX(iy1, 0);
                                  iy2 = MIN(iy2, gydim - 1);

                                  /* process if in region of interest */
                                    for (ii=ix1;ii<=ix2;ii++)
                                     for (jj=iy1;jj<=iy2;jj++)
                                        {
                                        /* set grid if highest weight */
                                        kgrid = ii*gydim + jj;
                                        xx = dx * ii + wbnd[0];
                                        yy = dy * jj + wbnd[2];
                                        inside = mb_pr_point_in_quad(verbose, xx, yy,
                                                                    footprints[ib].x, footprints[ib].y,
                                                                    &error);
                                        if (inside == MB_YES
                                            && priorities[ib] > maxpriority[kgrid])
                                            {
                                            grid[kgrid] = ss[ib];
                                            cnt[kgrid] = 1;
                                            maxpriority[kgrid] = priorities[ib];
                                            }
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
			fprintf(outfp,"%u data points processed in %s\n",
				ndatafile,file);

		/* add to datalist if data actually contributed */
		if (grid_mode != MBMOSAIC_AVERAGE
			&& ndatafile > 0 && dfp != NULL)
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
		fprintf(outfp,"\n%u total data points processed in highest weight pass\n",ndata);
	if (verbose > 0 && grid_mode == MBMOSAIC_AVERAGE)
		fprintf(outfp, "\n");

	}
	/***** end of first pass gridding *****/

	/***** do second pass gridding *****/
	if (grid_mode == MBMOSAIC_AVERAGE)
	{
	/* initialize arrays */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
			grid[kgrid] = 0.0;
			cnt[kgrid] = 0;
			sigma[kgrid] = 0.0;
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

		/* if format > 0 then input is multibeam file */
		if (format > 0 && file[0] != '#')
		{
		/* apply pstatus */
		if (pstatus == MB_PROCESSED_USE)
			strcpy(file, ppath);
		else
			strcpy(file, path);

		/* check for mbinfo file - get file bounds if possible */
		status = mb_check_info(verbose, file, lonflip, bounds,
				&file_in_bounds, &error);
		if (status == MB_FAILURE)
			{
			file_in_bounds = MB_YES;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* initialize the multibeam file */
		if (file_in_bounds == MB_YES)
		    {
		    /* check for filtered amplitude or sidescan file */
		    if (usefiltered == MB_YES && datatype == MBMOSAIC_DATA_AMPLITUDE)
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
		    else if (usefiltered == MB_YES && datatype == MBMOSAIC_DATA_SIDESCAN)
			{
			if ((status = mb_get_ffs(verbose, file, &format, &error)) != MB_SUCCESS)
			    {
			    mb_error(verbose,error,&message);
			    fprintf(stderr,"\nMBIO Error returned from function <mb_get_ffa>:\n%s\n",message);
			    fprintf(stderr,"Requested filtered sidescan file missing\n");
			    fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
			    fprintf(stderr,"\nProgram <%s> Terminated\n",
				    program_name);
			    exit(error);
			    }
			}

		    /* open the file */
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
			mb_memory_clear(verbose, &error);
			exit(error);
			}

                    /* get pointers to data storage */
                    mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
                    store_ptr = mb_io_ptr->store_data;

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
							    sizeof(double), (void **)&bathacrosstrack, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathalongtrack, &error);
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
							    sizeof(double), (void **)&ssacrosstrack, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&ssalongtrack, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslon, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&sslat, &error);
		    if (datatype != MBMOSAIC_DATA_SIDESCAN)
		    	{
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&gangles, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&priorities, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(struct footprint), (void **)&footprints, &error);
			}
		    else
		    	{
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&gangles, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&priorities, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(struct footprint), (void **)&footprints, &error);
			}
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&work1, &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&work2, &error);

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
                        status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
                                            time_i,&time_d,&navlon,&navlat,
                                            &speed,&heading,
                                            &distance,&altitude,&sonardepth,
                                            &beams_bath,&beams_amp,&pixels_ss,
                                            beamflag,bath,amp,bathacrosstrack,bathalongtrack,
                                            ss,ssacrosstrack,ssalongtrack,
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

			if (status == MB_SUCCESS && kind == MB_DATA_DATA)
                            {
                            status = mb_extract_nav(verbose,mbio_ptr,store_ptr,&kind,
                                                    time_i,&time_d,&navlon,&navlat,
                                                    &speed,&heading,&draft,
                                                    &roll,&pitch,&heave,&error);

                            /* get factors for lon lat calculations */
                            if (error == MB_ERROR_NO_ERROR)
                                    {
                                    mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
                                    headingx = sin(DTR*heading);
                                    headingy = cos(DTR*heading);
                                    }

                            /* get beam widths */
                            if (error == MB_ERROR_NO_ERROR)
                                    {
                                    status = mb_beamwidths(verbose, mbio_ptr, &beamwidth_xtrack, &beamwidth_ltrack, &error);
                                    }

                            /* mosaic beam based data (amplitude, grazing angle, slope) */
                            if (use_beams == MB_YES
                                    && error == MB_ERROR_NO_ERROR)
                              {

                              /* translate beam locations to lon/lat */
                              for (ib=0;ib<beams_amp;ib++)
                                {
                                if (mb_beam_ok(beamflag[ib]))
                                    {
                                    bathlon[ib] = navlon
                                        + headingy * mtodeglon
                                            * bathacrosstrack[ib]
                                        + headingx * mtodeglon
                                            * bathalongtrack[ib];
                                    bathlat[ib] = navlat
                                        - headingx * mtodeglat
                                            * bathacrosstrack[ib]
                                        + headingy * mtodeglat
                                            * bathalongtrack[ib];

                                    /* get footprints */
                                    mbmosaic_get_footprint(verbose, MBMOSAIC_FOOTPRINT_REAL,
                                                            beamwidth_xtrack, beamwidth_ltrack,
                                                            (bath[ib] - sonardepth),
                                                            bathacrosstrack[ib], bathalongtrack[ib],
                                                            0.0, &footprints[ib], &error);
                                    for (j=0;j<4;j++)
                                            {
                                            xx = navlon
                                                + headingy * mtodeglon
                                                    * footprints[ib].x[j]
                                                + headingx * mtodeglon
                                                    * footprints[ib].y[j];
                                            yy = navlat
                                                - headingx * mtodeglat
                                                    * footprints[ib].x[j]
                                                + headingy * mtodeglat
                                                    * footprints[ib].y[j];
                                            footprints[ib].x[j] = xx;
                                            footprints[ib].y[j] = yy;
                                            }
                                    }
                                }

                              /* get beam angles */
                              mbmosaic_get_beamangles(verbose, sonardepth, beams_bath, beamflag, bath, bathacrosstrack, bathalongtrack,
                                                      gangles, &error);

                              /* get priorities */
                              mbmosaic_get_beampriorities(verbose, priority_mode,
                                    n_priority_angle, priority_angle_angle, priority_angle_priority,
                                    priority_azimuth, priority_azimuth_factor,
                                    heading, beams_bath, beamflag, gangles, priorities, &error);

                              /* get bathymetry slopes if needed */
                              if (use_slope == MB_YES)
                                    mbmosaic_get_beamslopes(verbose,beams_bath,beamflag,bath,bathacrosstrack,slopes,&error);

                              /* reproject beam positions if necessary */
                              if (use_projection == MB_YES)
                                {
                                for (ib=0;ib<beams_amp;ib++)
                                  if (mb_beam_ok(beamflag[ib]))
                                    {
                                    mb_proj_forward(verbose, pjptr,
                                                    bathlon[ib], bathlat[ib],
                                                    &bathlon[ib], &bathlat[ib],
                                                    &error);
                                    for (j=0;j<4;j++)
                                            {
                                            mb_proj_forward(verbose, pjptr,
                                                    footprints[ib].x[j], footprints[ib].y[j],
                                                    &footprints[ib].x[j], &footprints[ib].y[j],
                                                    &error);
                                            }
                                    }
                                }

                              /* deal with data */
                              for (ib=0;ib<beams_amp;ib++)
                                if (mb_beam_ok(beamflag[ib]))
                                  {
                                  /* get position in grid */
                                  for (j=0;j<4;j++)
                                     {
                                     ixx[j] = (footprints[ib].x[j] - wbnd[0] + 0.5*dx)/dx;
                                     iyy[j] = (footprints[ib].y[j] - wbnd[2] + 0.5*dy)/dy;
                                     }
                                  ix1 = ixx[0];
                                  iy1 = iyy[0];
                                  ix2 = ixx[0];
                                  iy2 = iyy[0];
                                  for (j=1;j<4;j++)
                                     {
                                     ix1 = MIN(ix1, ixx[j]);
                                     iy1 = MIN(iy1, iyy[j]);
                                     ix2 = MAX(ix2, ixx[j]);
                                     iy2 = MAX(iy2, iyy[j]);
                                     }
    /*		              dix = (int)(scale * (ix2 - ix1));
                                  diy = (int)(scale * (iy2 - iy1));
                                  ix1 = MAX(ix1 - dix, 0);
                                  ix2 = MIN(ix2 + dix, gxdim - 1);
                                  iy1 = MAX(iy1 - diy, 0);
                                  iy2 = MIN(iy2 + diy, gydim - 1);*/
                                  ix1 = MAX(ix1, 0);
                                  ix2 = MIN(ix2, gxdim - 1);
                                  iy1 = MAX(iy1, 0);
                                  iy2 = MIN(iy2, gydim - 1);

                                  /* process if in region of interest */
                                    for (ii=ix1;ii<=ix2;ii++)
                                     for (jj=iy1;jj<=iy2;jj++)
                                        {
                                        /* add to cell if weight high enough */
                                        kgrid = ii*gydim + jj;
                                        xx = dx * ii + wbnd[0];
                                        yy = dy * jj + wbnd[2];
                                        inside = mb_pr_point_in_quad(verbose, xx, yy,
                                                                    footprints[ib].x, footprints[ib].y,
                                                                    &error);
                                        if (inside == MB_YES
                                            && priorities[ib] > 0.0
                                            && priorities[ib] >= maxpriority[kgrid] - priority_range)
                                            {
                                            xx = wbnd[0] + ii*dx - bathlon[ib];
                                            yy = wbnd[2] + jj*dy - bathlat[ib];
                                            norm_weight = file_weight * exp(-(xx*xx + yy*yy)*gaussian_factor);
                                            if (weight_priorities == 1)
                                                    norm_weight *= priorities[ib];
                                            else if (weight_priorities == 2)
                                                    norm_weight *= priorities[ib] * priorities[ib];
					    value = 0;
					    if (datatype == MBMOSAIC_DATA_AMPLITUDE)
					        {
					        value = amp[ib];
					        }
					    else if (datatype >= MBMOSAIC_DATA_OTHER)
					        {
					        get_other_data(mbio_ptr, ib, datatype, &value, &error);
					        }
					    else if (datatype == MBMOSAIC_DATA_FLAT_GRAZING)
						{
						if (gangles[ib] > 0)
						    value = gangles[ib];
						else
						    value = - gangles[ib];
						}
					    else if (datatype == MBMOSAIC_DATA_GRAZING)
					 	{
						slope += gangles[ib];
						if (slope < 0)
						    value = - slope;
						else
						    value = slope;
						}
					    else if (datatype == MBMOSAIC_DATA_SLOPE)
						{
						if (slope < 0)
						    value = - slope;
						else
						    value = slope;
						}

					    if (linear == MB_YES)
					        value = exp10(value*0.1);

                                            norm[kgrid] += norm_weight;
					    grid[kgrid] += norm_weight * value;
					    sigma[kgrid] += norm_weight * value * value;

                                            cnt[kgrid]++;
                                            }
                                        }
                                    ndata++;
                                    ndatafile++;
                                  }
                              }

                            /* mosaic sidescan */
                            else if (datatype == MBMOSAIC_DATA_SIDESCAN
                                    && error == MB_ERROR_NO_ERROR)
                              {
                              /* get spacing */
                              xsmin = 0.0;
                              xsmax = 0.0;
                              ismin = pixels_ss / 2;
                              ismax = pixels_ss / 2;
                              for (ib=0;ib<pixels_ss;ib++)
                                {
                                if (ss[ib] > MB_SIDESCAN_NULL)
                                    {
                                    if (ssacrosstrack[ib] < xsmin)
                                            {
                                            xsmin = ssacrosstrack[ib];
                                            ismin = ib;
                                            }
                                    if (ssacrosstrack[ib] > xsmax)
                                            {
                                            xsmax = ssacrosstrack[ib];
                                            ismax = ib;
                                            }
                                    }
                                }
                              if (ismax > ismin)
                                    {
                                    footprint_mode = MBMOSAIC_FOOTPRINT_SPACING;
                                    acrosstrackspacing = (xsmax - xsmin) / (ismax - ismin);
                                    }
                              else
                                    {
                                    footprint_mode = MBMOSAIC_FOOTPRINT_REAL;
                                    acrosstrackspacing = 0.0;
                                    }

                              /* translate pixel locations to lon/lat */
                              for (ib=0;ib<pixels_ss;ib++)
                                {
                                if (ss[ib] > MB_SIDESCAN_NULL)
                                    {
                                    sslon[ib] = navlon
                                        + headingy * mtodeglon
                                            * ssacrosstrack[ib]
                                        + headingx * mtodeglon
                                            * ssalongtrack[ib];
                                    sslat[ib] = navlat
                                        - headingx * mtodeglat
                                            * ssacrosstrack[ib]
                                        + headingy * mtodeglat
                                            * ssalongtrack[ib];

                                    /* get footprints */
                                    mbmosaic_get_footprint(verbose, footprint_mode,
                                                            beamwidth_xtrack, beamwidth_ltrack,
                                                            altitude,
                                                            ssacrosstrack[ib], ssalongtrack[ib],
                                                            acrosstrackspacing, &footprints[ib], &error);
                                    for (j=0;j<4;j++)
                                            {
                                            xx = navlon
                                                + headingy * mtodeglon
                                                    * footprints[ib].x[j]
                                                + headingx * mtodeglon
                                                    * footprints[ib].y[j];
                                            yy = navlat
                                                - headingx * mtodeglat
                                                    * footprints[ib].x[j]
                                                + headingy * mtodeglat
                                                    * footprints[ib].y[j];
                                            footprints[ib].x[j] = xx;
                                            footprints[ib].y[j] = yy;
                                            }
                                    }
                                }

                              /* get angle vs acrosstrack distance table using topographic grid */
                              table_error = MB_ERROR_NO_ERROR;
                              if (usetopogrid == MB_YES)
                                {
                                table_status = mb_topogrid_getangletable(verbose, topogrid_ptr, nangle, angle_min, angle_max,
                                                                            navlon, navlat, heading,
                                                                            altitude, sonardepth, pitch,
                                                                            table_angle, table_xtrack, table_ltrack,
                                                                            table_altitude, table_range, &table_error);
                                if (table_status == MB_FAILURE)
                                    {
                                    mb_error(verbose,table_error,&message);
                                    fprintf(outfp,"\nMBIO Error allocating data arrays:\n%s\n",message);
                                    fprintf(outfp,"\nNonfatal error in program <%s>\n",program_name);
                                    fprintf(outfp,"Sidescan data extends beyond the bounds of the topography grid <%s>\n",topogridfile);
                                    fprintf(outfp,"used for grazing angle calculation - the mosaic may be truncated.\n");
                                    table_status = MB_SUCCESS;
                                    table_error = MB_ERROR_NO_ERROR;
                                    }
                                }

                              /* get angle vs acrosstrack distance table using bathymetry from the swath file with sidescan */
                              else
                                {
                                table_status = mbmosaic_bath_getangletable(verbose, sonardepth, beams_bath, beamflag, bath, bathacrosstrack, bathalongtrack,
                                                                            angle_min, angle_max, nangle, table_angle, table_xtrack, table_ltrack,
                                                                            table_altitude, table_range, &table_error);
                                }

                              /* if need be, calculate angles using flat bottom layout and nadir altitude */
                              if (table_status == MB_FAILURE)
                                {
                                if (altitude <= 0.0)
                                    altitude = altitude_default;
                                table_status = mbmosaic_flatbottom_getangletable(verbose, altitude,
                                                                            angle_min, angle_max, nangle, table_angle, table_xtrack, table_ltrack,
                                                                            table_altitude, table_range, &table_error);
                                }

                              /* get angles for each pixel */
                              mbmosaic_get_ssangles(verbose, nangle, table_angle, table_xtrack, table_ltrack, table_altitude, table_range,
                                                                            pixels_ss, ss, ssacrosstrack, gangles, &error);

                              /* get priorities for each pixel */
                              mbmosaic_get_sspriorities(verbose, priority_mode,
                                    n_priority_angle, priority_angle_angle, priority_angle_priority,
                                    priority_azimuth, priority_azimuth_factor,
                                    heading, pixels_ss, ss, gangles, priorities, &error);

                            /* reproject pixel positions if necessary */
                            if (use_projection == MB_YES)
                              {
                              for (ib=0;ib<pixels_ss;ib++)
                                if (ss[ib] > MB_SIDESCAN_NULL)
                                  {
                                  mb_proj_forward(verbose, pjptr,
                                                  sslon[ib], sslat[ib],
                                                  &sslon[ib], &sslat[ib],
                                                  &error);
                                  for (j=0;j<4;j++)
                                          {
                                          mb_proj_forward(verbose, pjptr,
                                                  footprints[ib].x[j], footprints[ib].y[j],
                                                  &footprints[ib].x[j], &footprints[ib].y[j],
                                                  &error);
                                          }
                                  }
                              }

                            /* deal with data */
                            for (ib=0;ib<pixels_ss;ib++)
                              if (ss[ib] > MB_SIDESCAN_NULL)
                                {
                                /* get position in grid */
                                for (j=0;j<4;j++)
                                   {
                                   ixx[j] = (footprints[ib].x[j] - wbnd[0] + 0.5*dx)/dx;
                                   iyy[j] = (footprints[ib].y[j] - wbnd[2] + 0.5*dy)/dy;
                                   }
                                ix1 = ixx[0];
                                iy1 = iyy[0];
                                ix2 = ixx[0];
                                iy2 = iyy[0];
                                for (j=1;j<4;j++)
                                   {
                                   ix1 = MIN(ix1, ixx[j]);
                                   iy1 = MIN(iy1, iyy[j]);
                                   ix2 = MAX(ix2, ixx[j]);
                                   iy2 = MAX(iy2, iyy[j]);
                                   }
  /*		              dix = (int)(scale * (ix2 - ix1));
                                diy = (int)(scale * (iy2 - iy1));
                                ix1 = MAX(ix1 - dix, 0);
                                ix2 = MIN(ix2 + dix, gxdim - 1);
                                iy1 = MAX(iy1 - diy, 0);
                                iy2 = MIN(iy2 + diy, gydim - 1);*/
                                ix1 = MAX(ix1, 0);
                                ix2 = MIN(ix2, gxdim - 1);
                                iy1 = MAX(iy1, 0);
                                iy2 = MIN(iy2, gydim - 1);

                                /* process if in region of interest */
                                  for (ii=ix1;ii<=ix2;ii++)
                                   for (jj=iy1;jj<=iy2;jj++)
                                      {
                                      /* set grid if highest weight */
                                      kgrid = ii*gydim + jj;
                                      xx = dx * ii + wbnd[0];
                                      yy = dy * jj + wbnd[2];
                                      inside = mb_pr_point_in_quad(verbose, xx, yy,
                                                                  footprints[ib].x, footprints[ib].y,
                                                                  &error);
  /* fprintf(stderr,"priorities[%d]:%f maxpriority[%d]:%f range:%f",
  ib,priorities[ib],kgrid,maxpriority[kgrid],priority_range); */
                                      if (inside == MB_YES
                                          && priorities[ib] > 0.0
                                          && priorities[ib] >= maxpriority[kgrid] - priority_range)
                                          {
  /*fprintf(stderr," - USE DATA!"); */
                                          xx = wbnd[0] + ii*dx - sslon[ib];
                                          yy = wbnd[2] + jj*dy - sslat[ib];
                                          norm_weight = file_weight * exp(-(xx*xx + yy*yy)*gaussian_factor);
                                          if (weight_priorities == 1)
                                                  norm_weight *= priorities[ib];
                                          else if (weight_priorities == 2)
                                                  norm_weight *= priorities[ib] * priorities[ib];
					  if (linear == MB_NO)
					      value = ss[ib];
					  else
					      value = exp(ss[ib]*0.1);
 					  norm[kgrid] += norm_weight;
					  grid[kgrid] += norm_weight * value;
					  sigma[kgrid] += norm_weight * value * value;

                                          cnt[kgrid]++;
  /*fprintf(stderr," kgrid:%d norm_weight:%g grid:%g norm:%g cnt:%d",
  kgrid,norm_weight,grid[kgrid],norm[kgrid],cnt[kgrid]);*/
                                          }
  /* fprintf(stderr,"\n"); */
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
			fprintf(outfp,"%u data points processed in %s\n",
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
		} /* end if (format > 0) */

		}
	if (datalist != NULL)
		mb_datalist_close(verbose,&datalist,&error);
	if (verbose > 0)
		fprintf(outfp,"\n%u total data points processed in averaging pass\n",ndata);

	}
	/***** end of second pass gridding *****/

	/* close datalist if necessary */
	if (dfp != NULL)
		fclose(dfp);

	/* deallocate topography grid array if necessary */
	if (usetopogrid == MB_YES)
		status = mb_topogrid_deall(verbose, &topogrid_ptr, &error);

	/* now loop over all points in the output grid */
	if (verbose >= 1)
		fprintf(outfp,"\nMaking raw grid...\n");
	nbinset = 0;
	nbinzero = 0;
	nbinspline = 0;

	/* deal with single best mode */
	if (grid_mode == MBMOSAIC_SINGLE_BEST)
	    {
	    for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
		    {
		    kgrid = i*gydim + j;
		    if (cnt[kgrid] > 0)
			{
			nbinset++;
			}
		    else
			{
			grid[kgrid] = clipvalue;
			}
		    }
	    }
	else if (grid_mode == MBMOSAIC_AVERAGE)
	    {
	    for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
		    {
		    kgrid = i*gydim + j;
		    if (cnt[kgrid] > 0)
			{
			nbinset++;
			grid[kgrid] = grid[kgrid] / norm[kgrid];
			if (linear == MB_YES)
			    grid[kgrid] = 10 * log10(grid[kgrid]);
			sigma[kgrid] =
				sqrt(fabs(sigma[kgrid] / norm[kgrid]
					- grid[kgrid] * grid[kgrid]));
			}
		    else
			{
			grid[kgrid] = clipvalue;
			}
		    }
	    }

	/* if clip set do smooth interpolation */
	if (clipmode != MBMOSAIC_INTERP_NONE && clip > 0 && nbinset > 0)
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
				if (grid[kgrid] == clipvalue)
					{
					sdata[ndata++] = sxmin + dx*i;
					sdata[ndata++] = symin + dy*j;
					sdata[ndata++] = border;
					}
				j = gydim - 1;
				kgrid = i*gydim + j;
				if (grid[kgrid] == clipvalue)
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
				if (grid[kgrid] == clipvalue)
					{
					sdata[ndata++] = sxmin + dx*i;
					sdata[ndata++] = symin + dy*j;
					sdata[ndata++] = border;
					}
				i = gxdim - 1;
				kgrid = i*gydim + j;
				if (grid[kgrid] == clipvalue)
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
			fprintf(outfp,"\nDoing spline interpolation with %u data points...\n",ndata);
		cay = tension;
		xmin = sxmin - 0.5 * dx;
		ymin = symin - 0.5 * dy;
		ddx = dx;
		ddy = dy;
		if (clipmode == MBMOSAIC_INTERP_ALL)
			clip = MAX(gxdim,gydim);
		mb_zgrid2(sgrid,&gxdim,&gydim,&xmin,&ymin,
			&ddx,&ddy,sdata,&ndata,
			work1,work2,work3,&cay,&clip);

		if (clipmode == MBMOSAIC_INTERP_GAP)
		    fprintf(outfp,"Applying spline interpolation to fill gaps of %d cells or less...\n",clip);
		else if (clipmode == MBMOSAIC_INTERP_NEAR)
		    fprintf(outfp,"Applying spline interpolation to fill %d cells from data...\n",clip);
		else if (clipmode == MBMOSAIC_INTERP_ALL)
		    fprintf(outfp,"Applying spline interpolation to fill all undefined cells in the grid...\n");

		/* translate the interpolation into the grid array
		    filling only data gaps */
		zflag = 5.0e34;
		if (clipmode == MBMOSAIC_INTERP_GAP)
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
		else if (clipmode == MBMOSAIC_INTERP_NEAR)
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
		mb_freed(verbose,__FILE__,__LINE__,(void **)&sdata,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&sgrid,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&work1,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&work2,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&work3,&error);
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
				&& cnt[kgrid] > 1)
				smin = sigma[kgrid];
			if (smax == 0.0
				&& cnt[kgrid] > 1)
				smax = sigma[kgrid];
			if (sigma[kgrid] < smin && cnt[kgrid] > 1)
				smin = sigma[kgrid];
			if (sigma[kgrid] > smax && cnt[kgrid] > 1)
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
	if (datatype == MBMOSAIC_DATA_AMPLITUDE)
		{
		strcpy(zlabel,"Amplitude");
		strcpy(nlabel,"Number of Amplitude Data Points");
		strcpy(sdlabel,"Amplitude Standard Deviation (m)");
		strcpy(title,"Amplitude Grid");
		}
	else if (datatype == MBMOSAIC_DATA_SIDESCAN)
		{
		strcpy(zlabel,"Sidescan");
		strcpy(nlabel,"Number of Sidescan Data Points");
		strcpy(sdlabel,"Sidescan Standard Deviation (m)");
		strcpy(title,"Sidescan Grid");
		}
	else if (datatype == MBMOSAIC_DATA_FLAT_GRAZING)
		{
		strcpy(zlabel,"Degrees");
		strcpy(nlabel,"Number of Bottom Data Points");
		strcpy(sdlabel,"Grazing angle Standard Deviation (m)");
		strcpy(title,"Flat bottom grazing angle Grid");
		}
	else if (datatype == MBMOSAIC_DATA_GRAZING)
		{
		strcpy(zlabel,"Degrees");
		strcpy(nlabel,"Number of Bottom Data Points");
		strcpy(sdlabel,"Grazing angle Standard Deviation (m)");
		strcpy(title,"Grazing Angle Grid");
		}
	else if (datatype == MBMOSAIC_DATA_SLOPE)
		{
		strcpy(zlabel,"Degrees");
		strcpy(nlabel,"Number of Slope Data Points");
		strcpy(sdlabel,"Slope Standard Deviation (m)");
		strcpy(title,"Slope Grid");
		}
	else if (datatype >= MBMOSAIC_DATA_OTHER)
		{
		strcpy(zlabel,"Degrees");
		strcpy(nlabel,"Number of Data Points");
		strcpy(sdlabel,"Standard Deviation (m)");
		strcpy(title,"Grid");
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
			if (gridkind != MBMOSAIC_ASCII
				&& gridkind != MBMOSAIC_ARCASCII
				&& grid[kgrid] == clipvalue)
				{
				output[kout] = outclipvalue;
				}
			}
	if (gridkind == MBMOSAIC_ASCII)
		{
		strcpy(ofile,fileroot);
		strcat(ofile,".asc");
		status = write_ascii(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],
			dx,dy,&error);
		}
	else if (gridkind == MBMOSAIC_ARCASCII)
		{
		strcpy(ofile,fileroot);
		strcat(ofile,".asc");
		status = write_arcascii(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],
			dx,dy,clipvalue,&error);
		}
	else if (gridkind == MBMOSAIC_OLDGRD)
		{
		strcpy(ofile,fileroot);
		strcat(ofile,".grd1");
		status = write_oldgrd(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],dx,dy,&error);
		}
	else if (gridkind == MBMOSAIC_CDFGRD)
		{
		strcpy(ofile,fileroot);
		strcat(ofile,".grd");
		status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],
			zmin,zmax,dx,dy,
			xlabel,ylabel,zlabel,title,projection_id,
			argc,argv,&error);
		}
	else if (gridkind == MBMOSAIC_GMTGRD)
		{
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
		fprintf(stderr,"\nError writing output file: %s\n%s\n",
			ofile,message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
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
				if (gridkind != MBMOSAIC_ASCII
					&& gridkind != MBMOSAIC_ARCASCII
					&& cnt[kgrid] <= 0)
					output[kout] = outclipvalue;
				}
		if (gridkind == MBMOSAIC_ASCII)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_num.asc");
			status = write_ascii(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBMOSAIC_ARCASCII)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,".asc");
			status = write_arcascii(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,clipvalue,&error);
			}
		else if (gridkind == MBMOSAIC_OLDGRD)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_num.grd1");
			status = write_oldgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBMOSAIC_CDFGRD)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_num.grd");
			status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				zmin,zmax,dx,dy,
				xlabel,ylabel,nlabel,title,projection_id,
				argc,argv,&error);
			}
		else if (gridkind == MBMOSAIC_GMTGRD)
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
			fprintf(stderr,"\nError writing output file: %s\n%s\n",
				ofile,message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
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
				if (gridkind != MBMOSAIC_ASCII
					&& gridkind != MBMOSAIC_ARCASCII
					&& cnt[kgrid] <= 0)
					output[kout] = outclipvalue;
				}
		if (gridkind == MBMOSAIC_ASCII)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_sd.asc");
			status = write_ascii(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBMOSAIC_ARCASCII)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,".asc");
			status = write_arcascii(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,clipvalue,&error);
			}
		else if (gridkind == MBMOSAIC_OLDGRD)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_sd.grd1");
			status = write_oldgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBMOSAIC_CDFGRD)
			{
			strcpy(ofile,fileroot);
			strcat(ofile,"_sd.grd");
			status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				zmin,zmax,dx,dy,
				xlabel,ylabel,sdlabel,title,projection_id,
				argc,argv,&error);
			}
		else if (gridkind == MBMOSAIC_GMTGRD)
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
			fprintf(stderr,"\nError writing output file: %s\n%s\n",
				ofile,message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		}

	/* deallocate arrays */
	mb_freed(verbose,__FILE__,__LINE__,(void **)&grid,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&norm,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&maxpriority,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&cnt,&error);
	if (clip != 0)
	    mb_freed(verbose,__FILE__,__LINE__,(void **)&num,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&sigma,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&output,&error);
	if (priority_source == MBMOSAIC_PRIORITYTABLE_FILE
		&& n_priority_angle > 0)
		{
		mb_freed(verbose,__FILE__,__LINE__,(void **)&priority_angle_angle,&error);
		mb_freed(verbose,__FILE__,__LINE__,(void **)&priority_angle_priority,&error);
		}

	/* deallocate projection */
	if (use_projection == MB_YES)
		proj_status = mb_proj_free(verbose, &(pjptr), &error);

	/* run mbm_grdplot */
	if (gridkind == MBMOSAIC_GMTGRD)
		{
		/* execute mbm_grdplot */
		strcpy(ofile,fileroot);
		strcat(ofile,".grd");
		if (datatype == MBMOSAIC_DATA_AMPLITUDE)
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
		&& gridkind == MBMOSAIC_GMTGRD)
		{
		/* execute mbm_grdplot */
		strcpy(ofile,fileroot);
		strcat(ofile,"_num.grd");
		sprintf(plot_cmd, "mbm_grdplot -I%s%s -G1 -W1/2 -V -L\"File %s - %s:%s\"",
			ofile, gridkindstring, ofile, title, nlabel);
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
		sprintf(plot_cmd, "mbm_grdplot -I%s%s -G1 -W1/2 -V -L\"File %s - %s:%s\"",
			ofile, gridkindstring, ofile, title, sdlabel);
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
	char	date[25], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
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
		fprintf(stderr,"dbg2       grid:       %lu\n",(size_t)grid);
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
		fprintf(fp,"grid created by program mbmosaic\n");
		right_now = time((time_t *)0);
		strncpy(date,"\0",25);
		strncpy(date,ctime(&right_now),24);
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
		fprintf(stderr,"dbg2       grid:       %lu\n",(size_t)grid);
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
		fprintf(stderr,"dbg2       grid:       %lu\n",(size_t)grid);
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
#ifdef GMT_MINOR_VERSION
	GMT_LONG	pad[4];
#else
	int	pad[4];
#endif
	float	*a;
	time_t	right_now;
	char	date[MB_PATH_MAXLINE], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
	char	remark[MB_PATH_MAXLINE];
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
		fprintf(stderr,"dbg2       grid:       %lu\n",(size_t)grid);
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
		fprintf(stderr,"dbg2       *argv:      %lu\n",(size_t)*argv);
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
	strncpy(date,"\0",MB_PATH_MAXLINE);
	right_now = time((time_t *)0);
	strncpy(date,ctime(&right_now),24);
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
int double_compare(double *a, double *b)
{
	if (*a > *b)
		return(1);
	else
		return(-1);
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_footprint(
		int	verbose,
		int	mode,
		double	beamwidth_xtrack,
		double	beamwidth_ltrack,
		double	altitude,
		double	acrosstrack,
		double	alongtrack,
		double	acrosstrack_spacing,
		struct footprint *footprint,
		int	*error)
{
	char	*function_name = "mbmosaic_get_footprint";
	int	status = MB_SUCCESS;
	double	r;
	double	theta, phi, thetap, phip;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:             %d\n",verbose);
		fprintf(stderr,"dbg2       mode:                %d\n",mode);
		fprintf(stderr,"dbg2       beamwidth_xtrack:    %f\n",beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:    %f\n",beamwidth_ltrack);
		fprintf(stderr,"dbg2       altitude:            %f\n",altitude);
		fprintf(stderr,"dbg2       acrosstrack:         %f\n",acrosstrack);
		fprintf(stderr,"dbg2       alongtrack:          %f\n",alongtrack);
		fprintf(stderr,"dbg2       acrosstrack_spacing: %f\n",acrosstrack_spacing);
		}

	/* calculate footprint location in sonar coordinates */
	r = sqrt(altitude * altitude + acrosstrack * acrosstrack + alongtrack * alongtrack);
	mb_xyz_to_takeoff(verbose, acrosstrack, alongtrack, altitude, &theta, &phi, error);

	phip = phi - 0.5 * beamwidth_ltrack;
	thetap = theta - 0.5 * beamwidth_xtrack;
	if (mode == MBMOSAIC_FOOTPRINT_REAL)
		footprint->x[0] = r * sin(DTR * thetap) * cos(DTR * phip);
	else
		footprint->x[0] = acrosstrack - 0.5 * acrosstrack_spacing;
	footprint->y[0] = r * sin(DTR * thetap) * sin(DTR * phip);

	phip = phi - 0.5 * beamwidth_ltrack;
	thetap = theta + 0.5 * beamwidth_xtrack;
	if (mode == MBMOSAIC_FOOTPRINT_REAL)
		footprint->x[1] = r * sin(DTR * thetap) * cos(DTR * phip);
	else
		footprint->x[1] = acrosstrack + 0.5 * acrosstrack_spacing;
	footprint->y[1] = r * sin(DTR * thetap) * sin(DTR * phip);

	phip = phi + 0.5 * beamwidth_ltrack;
	thetap = theta + 0.5 * beamwidth_xtrack;
	if (mode == MBMOSAIC_FOOTPRINT_REAL)
		footprint->x[2] = r * sin(DTR * thetap) * cos(DTR * phip);
	else
		footprint->x[2] = acrosstrack + 0.5 * acrosstrack_spacing;
	footprint->y[2] = r * sin(DTR * thetap) * sin(DTR * phip);

	phip = phi + 0.5 * beamwidth_ltrack;
	thetap = theta - 0.5 * beamwidth_xtrack;
	if (mode == MBMOSAIC_FOOTPRINT_REAL)
		footprint->x[3] = r * sin(DTR * thetap) * cos(DTR * phip);
	else
		footprint->x[3] = acrosstrack - 0.5 * acrosstrack_spacing;
	footprint->y[3] = r * sin(DTR * thetap) * sin(DTR * phip);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		for (i=0;i<4;i++)
			fprintf(stderr,"dbg2       footprint: x[%d]:%f y[%d]:%f\n",i,footprint->x[i],i,footprint->y[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_beamangles(
		int     verbose,
                double  sonardepth,
                int     beams_bath,
                char    *beamflag,
                double  *bath,
                double  *bathacrosstrack,
                double  *bathalongtrack,
                double  *gangles,
                int     *error)
{
	char	*function_name = "mbmosaic_get_beamangles";
	int	status = MB_SUCCESS;
 	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       sonardepth:      %f\n",sonardepth);
		fprintf(stderr,"dbg2       beams_bath:      %d\n",beams_bath);
		fprintf(stderr,"dbg2       bathymetry:\n");
		for (i=0;i<beams_bath;i++)
			fprintf(stderr,"dbg2         beam:%d  flag:%d  bath:%f %f %f\n",
				i, beamflag[i], bath[i], bathacrosstrack[i], bathalongtrack[i]);
		}

        /* loop over all beams, calculate grazing angles for valid beams */
        for (i=0;i<beams_bath;i++)
                {
                if (mb_beam_ok(beamflag[i]))
                        {
                        gangles[i] = RTD * atan(bathacrosstrack[i] / (bath[i] - sonardepth));
                        }
                }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       beams_bath:      %d\n",beams_bath);
		fprintf(stderr,"dbg2       bathymetry:\n");
		for (i=0;i<beams_bath;i++)
			fprintf(stderr,"dbg2         beam:%d  flag:%d  bath:%f %f %f  angle:%f\n",
				i, beamflag[i], bath[i], bathacrosstrack[i], bathalongtrack[i], gangles[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_beampriorities(
                int     verbose,
                int     priority_mode,
                int     n_priority_angle,
                double  *priority_angle_angle,
                double  *priority_angle_priority,
                double  priority_azimuth,
                double  priority_azimuth_factor,
                double  heading,
                int     beams_bath,
                char    *beamflag,
                double  *gangles,
                double  *priorities,
                int     *error)
{
	char	*function_name = "mbmosaic_get_beampriorities";
	int	status = MB_SUCCESS;
        double  azi_starboard, azi_port, weight_starboard, weight_port;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n",verbose);
		fprintf(stderr,"dbg2       priority_mode:             %d\n",priority_mode);
		fprintf(stderr,"dbg2       n_priority_angle:          %d\n",n_priority_angle);
		fprintf(stderr,"dbg2       priority angle table:\n");
		for (i=0;i<n_priority_angle;i++)
			fprintf(stderr,"dbg2         %d  angle:%f  priority:%f\n",
				i, priority_angle_angle[i], priority_angle_priority[i]);
		fprintf(stderr,"dbg2       priority_azimuth:          %f\n",priority_azimuth);
		fprintf(stderr,"dbg2       priority_azimuth_factor:   %f\n",priority_azimuth_factor);
		fprintf(stderr,"dbg2       heading:         %f\n",heading);
		fprintf(stderr,"dbg2       beams_bath:      %d\n",beams_bath);
		fprintf(stderr,"dbg2       bathymetry grazing angles:\n");
		for (i=0;i<beams_bath;i++)
			fprintf(stderr,"dbg2         beam:%d  flag:%d angle:%f\n",
				i, beamflag[i], gangles[i]);
		}

	/* initialize priority array */
	for (i=0;i<beams_bath;i++)
            {
            if (mb_beam_ok(beamflag[i]))
                {
                priorities[i] = 1.0;
                }
            else
                {
                priorities[i] = 0.0;
                }
            }

	/* get grazing angle priorities */
	if (priority_mode == MBMOSAIC_PRIORITY_ANGLE
		|| priority_mode == MBMOSAIC_PRIORITY_BOTH)
            {
            /* loop over data getting angle based priorities */
            for (i=0;i<beams_bath;i++)
                {
                if (mb_beam_ok(beamflag[i]))
                    {
                    /* priority zero if outside the range of the priority-angle table */
                    if (gangles[i] < priority_angle_angle[0]
                        || gangles[i] > priority_angle_angle[n_priority_angle-1])
                        {
                        priorities[i] = 0.0;
                        }

                    /* priority set using the priority-angle table */
                    else
                        {
                        for (j=0;j<n_priority_angle-1;j++)
                            {
                            if (gangles[i] >= priority_angle_angle[j]
                                && gangles[i] < priority_angle_angle[j+1])
                                {
                                priorities[i] *= (priority_angle_priority[j]
                                                + (priority_angle_priority[j+1] - priority_angle_priority[j])
                                                    * (gangles[i] - priority_angle_angle[j])
                                                    / (priority_angle_angle[j+1] - priority_angle_angle[j]));
                                }
                            }
                        }
                    }
                }
            }

	/* get look azimuth priorities */
	if (priority_mode == MBMOSAIC_PRIORITY_AZIMUTH
		|| priority_mode == MBMOSAIC_PRIORITY_BOTH)
            {
            /* get priorities for starboard and port sides of ping */
            azi_starboard = heading - 90.0 - priority_azimuth;
            if (azi_starboard > 180.0)
                azi_starboard -= 360.0 * ((int) ((azi_starboard + 180.0) / 360.0));
            else if (azi_starboard < -180.0)
                azi_starboard += 360.0 * ((int) ((-azi_starboard + 180.0) / 360.0));
            if (priority_azimuth_factor * azi_starboard <= -90.0
                || priority_azimuth_factor * azi_starboard >= 90.0)
                weight_starboard = 0.0;
            else
                weight_starboard = MAX(cos(DTR * priority_azimuth_factor * azi_starboard), 0.0);
            azi_port = heading + 90.0 - priority_azimuth;
            if (azi_port > 180.0)
                azi_port -= 360.0 * ((int) ((azi_port + 180.0) / 360.0));
            else if (azi_port < -180.0)
                azi_port += 360.0 * ((int) ((-azi_port + 180.0) / 360.0));
            if (priority_azimuth_factor * azi_port <= -90.0
                || priority_azimuth_factor * azi_port >= 90.0)
                weight_port = 0.0;
            else
                weight_port = MAX(cos(DTR * priority_azimuth_factor * azi_port), 0.0);

            /* apply the look azimuth priorities */
            for (i=0;i<beams_bath;i++)
                {
                if (mb_beam_ok(beamflag[i]))
                    {
                    if (gangles[i] < 0.0)
                        priorities[i] *= weight_starboard;
                    else
                        priorities[i] *= weight_port;
                    }
                }
             }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       beams_bath:      %d\n",beams_bath);
		fprintf(stderr,"dbg2       bathymetry grazing angles and priorities:\n");
		for (i=0;i<beams_bath;i++)
			fprintf(stderr,"dbg2         beam:%d  flag:%d angle:%f  priority:%f\n",
				i, beamflag[i], gangles[i], priorities[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_beamslopes(
                int verbose,
                int     beams_bath,
                char    *beamflag,
                double  *bath,
                double  *bathacrosstrack,
                double  *slopes,
                int     *error)
{
	char	*function_name = "mbmosaic_get_beamslopes";
	int	status = MB_SUCCESS;
        int     found_pre, found_post;
        int     i0, i1;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       beams_bath:      %d\n",beams_bath);
		fprintf(stderr,"dbg2       bathymetry:\n");
		for (i=0;i<beams_bath;i++)
			fprintf(stderr,"dbg2         beam:%d  flag:%d  bath:%f %f\n",
				i, beamflag[i], bath[i], bathacrosstrack[i]);
		}

	/* get grazing angle priorities */
	/* loop over data getting angle based priorities */
        for (i=0;i<beams_bath;i++)
            {
            if (mb_beam_ok(beamflag[i]))
                {
                /* find previous good beam */
                found_pre = MB_NO;
                if (i > 0)
                    {
                    for (j=i-1; j>=0 && found_pre == MB_NO; j--)
                        {
                        if (mb_beam_ok(beamflag[j]))
                            {
                            found_pre = MB_YES;
                            i0 = j;
                            }
                        }
                    }

               /* find post good beam */
                found_post = MB_NO;
                if (i < beams_bath-1)
                    {
                    for (j=i+1; j<beams_bath && found_post == MB_NO; j++)
                        {
                        if (mb_beam_ok(beamflag[j]))
                            {
                            found_post = MB_YES;
                            i1 = j;
                            }
                        }
                    }

                /* calculate slope */
                if (found_pre == MB_YES && found_post == MB_YES)
                    {
                    if (bathacrosstrack[i1] != bathacrosstrack[i0])
                        slopes[i] = -(bath[i1] - bath[i0]) / (bathacrosstrack[i1] - bathacrosstrack[i0]);
                    else
                        slopes[i] = 0.0;
                    }
               else if (found_pre == MB_YES)
                    {
                    if (bathacrosstrack[i] != bathacrosstrack[i0])
                        slopes[i] = -(bath[i] - bath[i0]) / (bathacrosstrack[i] - bathacrosstrack[i0]);
                    else
                        slopes[i] = 0.0;
                    }
              else if (found_post == MB_YES)
                    {
                    if (bathacrosstrack[i1] != bathacrosstrack[i])
                        slopes[i] = -(bath[i1] - bath[i]) / (bathacrosstrack[i1] - bathacrosstrack[i]);
                    else
                        slopes[i] = 0.0;
                    }
              else
                    {
                    slopes[i] = 0.0;
                    }
                }
            }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       beams_bath:      %d\n",beams_bath);
		fprintf(stderr,"dbg2       bathymetry:\n");
		for (i=0;i<beams_bath;i++)
			fprintf(stderr,"dbg2         beam:%d  flag:%d  bath:%f %f  slope:%f\n",
				i, beamflag[i], bath[i], bathacrosstrack[i], slopes[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_bath_getangletable(
                int     verbose,
                double  sonardepth,
                int     beams_bath,
                char    *beamflag,
                double  *bath,
                double  *bathacrosstrack,
                double  *bathalongtrack,
                double  angle_min,
                double  angle_max,
                int     nangle,
                double  *table_angle,
                double  *table_xtrack,
                double  *table_ltrack,
		double  *table_altitude,
                double  *table_range,
                int     *error)
{
	char	*function_name = "mbmosaic_bath_getangletable";
	int	status = MB_SUCCESS;
        double  dangle, angle0, angle1, factor;
        int     found, foundnext;
	int	i, j, jj, jstart, jnext;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       sonardepth:      %f\n",sonardepth);
		fprintf(stderr,"dbg2       beams_bath:      %d\n",beams_bath);
		fprintf(stderr,"dbg2       bathymetry:\n");
		for (i=0;i<beams_bath;i++)
			fprintf(stderr,"dbg2         beam:%d  flag:%d  bath:%f %f\n",
				i, beamflag[i], bath[i], bathacrosstrack[i]);
		fprintf(stderr,"dbg2       angle_min:       %f\n",angle_min);
		fprintf(stderr,"dbg2       angle_max:       %f\n",angle_max);
		fprintf(stderr,"dbg2       nangle:          %d\n",nangle);
		}

        /* loop over the angles and figure out the other table values from the bathymetry */
	dangle = (angle_max - angle_min) / (nangle - 1);
        jstart = 0;
        *error = MB_ERROR_NO_ERROR;
	for (i=0;i<nangle;i++)
            {
            /* get angles in takeoff coordinates */
            table_angle[i] = angle_min + dangle * i;
            table_xtrack[i] = 0.0;
            table_ltrack[i] = 0.0;
            table_range[i] = 0.0;

            /* estimate the table values for this angle from the bathymetry */
            found = MB_NO;
            for (j=jstart;j<beams_bath-1 && found == MB_NO; j++)
                {
                /* check if this beam is valid */
                if (mb_beam_ok(beamflag[j]))
                    {
                   /* look for the next valid beam */
                    foundnext = MB_NO;
                    jnext = j;
                    for (jj=j+1;jj<beams_bath && foundnext == MB_NO;jj++)
                        {
                        if (mb_beam_ok(beamflag[jj]))
                            {
                            jnext = jj;
                            foundnext = MB_YES;
                            }
                        }

                    /* get the angle for beam j */
                    angle0 = RTD * atan(bathacrosstrack[j] / (bath[j] - sonardepth));
                    if (foundnext == MB_YES)
                        angle1 = RTD * atan(bathacrosstrack[jnext] / (bath[jnext] - sonardepth));

                    /* deal with angle to port of swath edge */
/* fprintf(stderr,"i:%d angle:%f angle0:%f j:%d jnext:%d foundnext:%d ",
i,table_angle[i],angle0,j,jnext,foundnext);
if (foundnext == MB_YES)
fprintf(stderr," angle1:%f ",angle1); */
                    if (table_angle[i] <= angle0)
                        {
                        table_altitude[i] = bath[j] - sonardepth;
                        table_xtrack[i] = table_altitude[i] * tan(DTR * table_angle[i]);
                        table_ltrack[i] = bathalongtrack[j];
                        table_range[i] = sqrt(table_altitude[i] * table_altitude[i]
                                        + table_xtrack[i] * table_xtrack[i]
                                        + table_ltrack[i] * table_ltrack[i]);
                        found = MB_YES;
                        jstart = j;
/* fprintf(stderr," A table: %f %f %f %f %f\n",
table_angle[i],table_altitude[i],table_xtrack[i],table_ltrack[i],table_range[i]); */
                        }

                    /* deal with angle to starboard of swath edge */
                    else if (foundnext == MB_NO)
                        {
                        table_altitude[i] = bath[j] - sonardepth;
                        table_xtrack[i] = table_altitude[i] * tan(DTR * table_angle[i]);
                        table_ltrack[i] = bathalongtrack[j];
                        table_range[i] = sqrt(table_altitude[i] * table_altitude[i]
                                        + table_xtrack[i] * table_xtrack[i]
                                        + table_ltrack[i] * table_ltrack[i]);
                        found = MB_YES;
                        jstart = j;
/* fprintf(stderr," B table: %f %f %f %f %f\n",
table_angle[i],table_altitude[i],table_xtrack[i],table_ltrack[i],table_range[i]);*/
                        }

                    /* deal with angle to starboard of swath edge */
                    else if (foundnext == MB_YES && table_angle[i] > angle1)
                        {
                        if (jnext == beams_bath - 1)
                            {
                            table_altitude[i] = bath[j] - sonardepth;
                            table_xtrack[i] = table_altitude[i] * tan(DTR * table_angle[i]);
                            table_ltrack[i] = bathalongtrack[j];
                            table_range[i] = sqrt(table_altitude[i] * table_altitude[i]
                                            + table_xtrack[i] * table_xtrack[i]
                                            + table_ltrack[i] * table_ltrack[i]);
                            found = MB_YES;
                            }
                        jstart = j;
/* fprintf(stderr," B table: %f %f %f %f %f\n",
table_angle[i],table_altitude[i],table_xtrack[i],table_ltrack[i],table_range[i]);*/
                        }

                    /* deal with angle between the two valid beams */
                    else if (foundnext == MB_YES && table_angle[i] >= angle0 && table_angle[i] <= angle1)
                        {
                        factor = (table_angle[i] - angle0) / (angle1 - angle0);
                        table_altitude[i] = (bath[j] - sonardepth) + factor * (bath[jnext] - bath[j]);
                        table_xtrack[i] = table_altitude[i] * tan(DTR * table_angle[i]);
                        table_ltrack[i] = bathalongtrack[j] + factor * (bathalongtrack[jnext] - bathalongtrack[j]);
                        table_range[i] = sqrt(table_altitude[i] * table_altitude[i]
                                        + table_xtrack[i] * table_xtrack[i]
                                        + table_ltrack[i] * table_ltrack[i]);
                        found = MB_YES;
                        jstart = j;
/* fprintf(stderr," C factor:%f x:%f %f %f table: %f %f %f %f %f\n",
factor,bathacrosstrack[j],table_xtrack[i],bathacrosstrack[jnext],
table_angle[i],table_altitude[i],table_xtrack[i],table_ltrack[i],table_range[i]);*/
                        }

                    /* else skip */
/* else
fprintf(stderr," SKIP\n");*/
                    }
                }

            /* set error if necessary */
            if (found == MB_NO)
                {
                status = MB_FAILURE;
                *error = MB_ERROR_NOT_ENOUGH_DATA;
                }
            }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nangle:          %d\n",nangle);
		fprintf(stderr,"dbg2       tables:\n");
		for (i=0;i<nangle;i++)
			fprintf(stderr,"dbg2         %d angle:%f  xtrack:%f ltrack:%f altitude:%f range:%f\n",
				i, table_angle[i], table_xtrack[i], table_ltrack[i], table_altitude[i], table_range[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_flatbottom_getangletable(
                int     verbose,
                double  altitude,
                double  angle_min,
                double  angle_max,
                int     nangle,
                double  *table_angle,
                double  *table_xtrack,
                double  *table_ltrack,
		double  *table_altitude,
                double  *table_range,
                int     *error)
{
	char	*function_name = "mbmosaic_flatbottom_getangletable";
	int	status = MB_SUCCESS;
        double  dangle;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       altitude:        %f\n",altitude);
		fprintf(stderr,"dbg2       angle_min:       %f\n",angle_min);
		fprintf(stderr,"dbg2       angle_max:       %f\n",angle_max);
		fprintf(stderr,"dbg2       nangle:          %d\n",nangle);
		}

        /* loop over the angles and figure out the other table values from the bathymetry */
	dangle = (angle_max - angle_min) / (nangle - 1);
        *error = MB_ERROR_NO_ERROR;
	for (i=0;i<nangle;i++)
            {
            /* get angles in takeoff coordinates */
            table_angle[i] = angle_min + dangle * i;
            table_xtrack[i] = altitude * tan(DTR * table_angle[i]);
            table_ltrack[i] = 0.0;
            table_range[i] = sqrt(altitude * altitude + table_xtrack[i] * table_xtrack[i]);
            }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nangle:          %d\n",nangle);
		fprintf(stderr,"dbg2       tables:\n");
		for (i=0;i<nangle;i++)
			fprintf(stderr,"dbg2         %d angle:%f  xtrack:%f ltrack:%f altitude:%f range:%f\n",
				i, table_angle[i], table_xtrack[i], table_ltrack[i], table_altitude[i], table_range[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_ssangles(
                int     verbose,
                int     nangle,
                double  *table_angle,
                double  *table_xtrack,
                double  *table_ltrack,
                double  *table_altitude,
                double  *table_range,
                int  pixels_ss,
                double  *ss,
                double  *ssacrosstrack,
                double  *gangles,
                int  *error)
{
	char	*function_name = "mbmosaic_get_ssangles";
	int	status = MB_SUCCESS;
        int     found;
	int	i, j, jstart;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n",verbose);
		fprintf(stderr,"dbg2       nangle:          %d\n",nangle);
		fprintf(stderr,"dbg2       tables:\n");
		for (i=0;i<nangle;i++)
			fprintf(stderr,"dbg2         %d angle:%f  xtrack:%f ltrack:%f altitude:%f range:%f\n",
				i, table_angle[i], table_xtrack[i], table_ltrack[i], table_altitude[i], table_range[i]);
		fprintf(stderr,"dbg2       pixels_ss:       %d\n",pixels_ss);
		fprintf(stderr,"dbg2       sidescan:\n");
		for (i=0;i<pixels_ss;i++)
			fprintf(stderr,"dbg2         pixel:%d  ss:%f %f\n",
				i, ss[i], ssacrosstrack[i]);
		}

        /* loop over the sidescan interpolating angles from the table on the basis of ssacrosstrack */
        jstart = 0;
        for (i=0;i<pixels_ss;i++)
            {
            /* get angles only for valid sidescan */
            if (ss[i] > MB_SIDESCAN_NULL)
                {
                found = MB_NO;
                for (j=jstart;j<nangle-1 && found == MB_NO;j++)
                    {
                    if (ssacrosstrack[i] < table_xtrack[j])
                        {
                        gangles[i] = table_angle[j];
                        found = MB_YES;
                        }
                    else if (ssacrosstrack[i] >= table_xtrack[j]
                            && ssacrosstrack[i] <= table_xtrack[j+1])
                        {
                        gangles[i] = table_angle[j] + (table_angle[j+1] - table_angle[j])
                                                        * (ssacrosstrack[i] - table_xtrack[j])
                                                        / (table_xtrack[j+1] - table_xtrack[j]);
/*fprintf(stderr,"i:%d j:%d %d angles: %f %f %f  x: %f %f %f\n",
i,j,j+1,table_angle[j],gangles[i],table_angle[j+1],table_xtrack[j],ssacrosstrack[i],table_xtrack[j+1]);*/
                        found = MB_YES;
                        jstart = j;
                        }
                    else if (ssacrosstrack[i] >= table_xtrack[j+1]
                            && j == nangle - 2)
                        {
                        gangles[i] = table_angle[j+1];
                        found = MB_YES;
                        }
                    }
                }

            /* zero angles for invalid sidescan */
            else
                {
                gangles[i] = 0.0;
                }
            }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       pixels_ss:       %d\n",pixels_ss);
		fprintf(stderr,"dbg2       sidescan grazing angles:\n");
		for (i=0;i<pixels_ss;i++)
			fprintf(stderr,"dbg2         pixel:%d  ss:%f %f angle:%f\n",
				i, ss[i], ssacrosstrack[i], gangles[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_sspriorities(
                int     verbose,
                int     priority_mode,
                int     n_priority_angle,
                double  *priority_angle_angle,
                double  *priority_angle_priority,
                double  priority_azimuth,
                double  priority_azimuth_factor,
                double  heading,
                int     pixels_ss,
                double  *ss,
                double  *gangles,
                double  *priorities,
                int     *error)
{
	char	*function_name = "mbmosaic_get_sspriorities";
	int	status = MB_SUCCESS;
        double  azi_starboard, azi_port, weight_starboard, weight_port;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n",verbose);
		fprintf(stderr,"dbg2       priority_mode:             %d\n",priority_mode);
		fprintf(stderr,"dbg2       n_priority_angle:          %d\n",n_priority_angle);
		fprintf(stderr,"dbg2       priority angle table:\n");
		for (i=0;i<n_priority_angle;i++)
			fprintf(stderr,"dbg2         %d  angle:%f  priority:%f\n",
				i, priority_angle_angle[i], priority_angle_priority[i]);
		fprintf(stderr,"dbg2       priority_azimuth:          %f\n",priority_azimuth);
		fprintf(stderr,"dbg2       priority_azimuth_factor:   %f\n",priority_azimuth_factor);
		fprintf(stderr,"dbg2       heading:         %f\n",heading);
		fprintf(stderr,"dbg2       pixels_ss:       %d\n",pixels_ss);
		fprintf(stderr,"dbg2       sidescan grazing angles:\n");
		for (i=0;i<pixels_ss;i++)
			fprintf(stderr,"dbg2         pixel:%d  ss:%f angle:%f\n",
				i, ss[i], gangles[i]);
		}

	/* initialize priority array */
	for (i=0;i<pixels_ss;i++)
            {
            if (ss[i] > MB_SIDESCAN_NULL)
                {
                priorities[i] = 1.0;
                }
            else
                {
                priorities[i] = 0.0;
                }
            }

	/* get grazing angle priorities */
	if (priority_mode == MBMOSAIC_PRIORITY_ANGLE
		|| priority_mode == MBMOSAIC_PRIORITY_BOTH)
            {
            /* loop over data getting angle based priorities */
            for (i=0;i<pixels_ss;i++)
                {
                if (ss[i] > MB_SIDESCAN_NULL)
                    {
                    /* priority zero if outside the range of the priority-angle table */
                    if (gangles[i] < priority_angle_angle[0]
                        || gangles[i] > priority_angle_angle[n_priority_angle-1])
                        {
                        priorities[i] = 0.0;
                        }

                    /* priority set using the priority-angle table */
                    else
                        {
                        for (j=0;j<n_priority_angle-1;j++)
                            {
                            if (gangles[i] >= priority_angle_angle[j]
                                && gangles[i] < priority_angle_angle[j+1])
                                {
                                priorities[i] *= (priority_angle_priority[j]
                                                + (priority_angle_priority[j+1] - priority_angle_priority[j])
                                                    * (gangles[i] - priority_angle_angle[j])
                                                    / (priority_angle_angle[j+1] - priority_angle_angle[j]));
                                }
                            }
                        }
                    }
                }
            }

	/* get look azimuth priorities */
	if (priority_mode == MBMOSAIC_PRIORITY_AZIMUTH
		|| priority_mode == MBMOSAIC_PRIORITY_BOTH)
            {
            /* get priorities for starboard and port sides of ping */
            azi_starboard = heading - 90.0 - priority_azimuth;
            if (azi_starboard > 180.0)
                azi_starboard -= 360.0 * ((int) ((azi_starboard + 180.0) / 360.0));
            else if (azi_starboard < -180.0)
                azi_starboard += 360.0 * ((int) ((-azi_starboard + 180.0) / 360.0));
            if (priority_azimuth_factor * azi_starboard <= -90.0
                || priority_azimuth_factor * azi_starboard >= 90.0)
                weight_starboard = 0.0;
            else
                weight_starboard = MAX(cos(DTR * priority_azimuth_factor * azi_starboard), 0.0);
            azi_port = heading + 90.0 - priority_azimuth;
            if (azi_port > 180.0)
                azi_port -= 360.0 * ((int) ((azi_port + 180.0) / 360.0));
            else if (azi_port < -180.0)
                azi_port += 360.0 * ((int) ((-azi_port + 180.0) / 360.0));
            if (priority_azimuth_factor * azi_port <= -90.0
                || priority_azimuth_factor * azi_port >= 90.0)
                weight_port = 0.0;
            else
                weight_port = MAX(cos(DTR * priority_azimuth_factor * azi_port), 0.0);

            /* apply the look azimuth priorities */
            for (i=0;i<pixels_ss;i++)
                {
                if (ss[i] > MB_SIDESCAN_NULL)
                    {
                    if (gangles[i] < 0.0)
                        priorities[i] *= weight_starboard;
                    else
                        priorities[i] *= weight_port;
                    }
                }
             }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBmosaic function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       pixels_ss:       %d\n",pixels_ss);
		fprintf(stderr,"dbg2       sidescan grazing angles and priorities:\n");
		for (i=0;i<pixels_ss;i++)
			fprintf(stderr,"dbg2         pixel:%d  angle:%f  priority:%f\n",
				i, gangles[i], priorities[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
/*
 * Get Other Data
 * This method is meant to be hacked to allow the gridding of any data value
 */
#include "../../include/mbsys_simrad2.h"


int get_other_data(void* mbio_ptr, int beam, int datatype, double* data, int* error) {
        char    *function_name = "get_other_data";
        int     status = MB_SUCCESS;
        int     i;

        *data = 0;

        struct mb_io_struct             *mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
        if (mb_io_ptr->format == MBF_EM300RAW || mb_io_ptr->format == MBF_EM300MBA)
            {
            struct mbsys_simrad2_struct *store_ptr = (struct mbsys_simrad2_struct *) mb_io_ptr->store_data;
            struct mbsys_simrad2_ping_struct *ping_ptr = store_ptr->ping;

            switch (datatype)
                {
                case 10:  // Surface sound speed
                    *data = ping_ptr->png_ssv * 0.1;
                    break;

                case 11: // Sound absorption
                    *data = ping_ptr->png_max_range * 0.01;
                    break;

                case 12: // Mode
                    *data = store_ptr->run_mode;
                    break;

                case 13: // Pulse length;
                    *data = ping_ptr->png_r_zero;
                    break;
                }
            }
        return status;
}
/*--------------------------------------------------------------------*/
