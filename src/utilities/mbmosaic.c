/*--------------------------------------------------------------------
 *    The MB-system:	mbmosaic.c	2/10/97
 *    $Id: mbmosaic.c,v 5.24 2006-09-11 18:55:54 caress Exp $
 *
 *    Copyright (c) 1997, 2000, 2002, 2003, 2006 by
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
 * $Log: not supported by cvs2svn $
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
#include <math.h>
#include <string.h>
#include <time.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"

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

/* prioritization mode */
#define	MBMOSAIC_PRIORITY_NONE		0
#define	MBMOSAIC_PRIORITY_ANGLE		1
#define	MBMOSAIC_PRIORITY_AZIMUTH		2
#define	MBMOSAIC_PRIORITY_BOTH		3

/* flag for no data in grid */
#define	NO_DATA_FLAG	99999

/* program identifiers */
static char rcs_id[] = "$Id: mbmosaic.c,v 5.24 2006-09-11 18:55:54 caress Exp $";
static char program_name[] = "mbmosaic";
static char help_message[] =  "mbmosaic is an utility used to mosaic amplitude or \nsidescan data contained in a set of swath sonar data files.  \nThis program uses one of four algorithms (gaussian weighted mean, \nmedian filter, minimum filter, maximum filter) to grid regions \ncovered by multibeam swaths and then fills in gaps between \nthe swaths (to the degree specified by the user) using a minimum\ncurvature algorithm.";
static char usage_message[] = "mbmosaic -Ifilelist -Oroot \
-Rwest/east/south/north [-Adatatype \n\
          -Bborder -Cclip -Dxdim/ydim -Edx/dy/units \n\
          -Fpriority_range -Ggridkind -H -Jprojection -Llonflip -M -N -Ppings \n\
          -Sspeed -Ttension -Uazimuth/factor -V -Wscale -Xextend \n\
          -Ypriority_file -Zbathdef]";

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
	char	file[MB_PATH_MAXLINE];
	int	file_in_bounds;
	void	*mbio_ptr = NULL;

	/* mbmosaic control variables */
	char	filelist[MB_PATH_MAXLINE];
	char	fileroot[MB_PATH_MAXLINE];
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
	char	units[MB_PATH_MAXLINE];
	int	clip = 0;
	int	grid_mode = MBMOSAIC_SINGLE_BEST;
	int	datatype = MBMOSAIC_DATA_SIDESCAN;
	char	gridkindstring[MB_PATH_MAXLINE];
	int	gridkind = MBMOSAIC_GMTGRD;
	int	more = MB_NO;
	int	use_NaN = MB_NO;
	double	clipvalue = NO_DATA_FLAG;
	float	outclipvalue = NO_DATA_FLAG;
	double	scale = 1.0;
	double	border = 0.0;
	double	extend = 0.0;
	double	tension = 1e10;
	int	priority_mode = MBMOSAIC_PRIORITY_NONE;
	double	priority_range = 0.0;
	double	priority_azimuth = 0.0;
	double	priority_azimuth_factor = 1.0;
	char	pfile[MB_PATH_MAXLINE];
	int	n_priority_angle = 0;
	double	*priority_angle_angle = NULL;
	double	*priority_angle_priority = NULL;
	double	bath_default = 1000.0;
	char	ifile[MB_PATH_MAXLINE];
	char	ofile[MB_PATH_MAXLINE];
	char	plot_cmd[MB_COMMENT_MAXLINE];
	int	plot_status;
	int	use_beams = MB_NO;
	int 	use_slope = MB_NO;

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
	double	*angles = NULL;
	double	*priorities = NULL;

	/* grid variables */
	double	gbnd[4], wbnd[4], obnd[4];
	double	xlon, ylat, xx, yy;
	double	factor;
	int	gxdim, gydim, offx, offy, xtradim;
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
	float	*work1 = NULL;
	float	*work2 = NULL;
	float	*work3 = NULL;
	unsigned int	ndata, ndatafile;
	double	zmin, zmax, zclip;
	int	nmax;
	double	smin, smax;
	int	nbinset, nbinzero, nbinspline;

	/* crosstrack slope values */
	double	angle, depth, slope;
	int	ndepths;
	double	*depths;
	double	*depthacrosstrack;
	int	nslopes;
	double	*slopes;
	double	*slopeacrosstrack;

	/* projected grid parameters */
	int	use_projection = MB_NO;
	int	projection_pars_f = MB_NO;
	double	reference_lon, reference_lat;
	int	utm_zone = 1;
	char	projection_type;
	char	projection_pars[MB_PATH_MAXLINE];
	char	projection_id[MB_PATH_MAXLINE];
	int	proj_status;
	void	*pjptr;
	double	p_lon_1, p_lon_2;
	double	p_lat_1, p_lat_2;
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
	float	zero = 0.0;
	float	NaN;

	/* other variables */
	FILE	*fp;
	char	buffer[MB_PATH_MAXLINE], *result;
	int	i, j, ii, jj, iii, jjj, kkk, n;
	int	i1, i2, j1, j2;
	double	r;
	int	dmask[9];
	int	kgrid, kout, kint, ib, ix, iy;
	int	ix1, ix2, iy1, iy2;
	int	nscan;
	double	norm_weight;

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
			sscanf (optarg,"%lf", &priority_range);
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
			sscanf (optarg,"%s", pfile);
			if (priority_mode == MBMOSAIC_PRIORITY_AZIMUTH)
			    priority_mode = MBMOSAIC_PRIORITY_BOTH;
			else
			    priority_mode = MBMOSAIC_PRIORITY_ANGLE;
			break;
		case 'Z':
		case 'z':
			sscanf (optarg,"%lf", &bath_default);
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
		fprintf(outfp,"dbg2       clip:                 %d\n",clip);
		fprintf(outfp,"dbg2       more:                 %d\n",more);
		fprintf(outfp,"dbg2       use_NaN:              %d\n",use_NaN);
		fprintf(outfp,"dbg2       data type:            %d\n",datatype);
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
		fprintf(outfp,"dbg2       pfile:                %s\n",pfile);
		fprintf(outfp,"dbg2       priority_azimuth:     %f\n",priority_azimuth);
		fprintf(outfp,"dbg2       priority_azimuth_fac: %f\n",priority_azimuth_factor);
		fprintf(outfp,"dbg2       bath_default:         %f\n",bath_default);
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

	/* if specified get static angle priorities */
	if (priority_mode == MBMOSAIC_PRIORITY_ANGLE
		|| priority_mode == MBMOSAIC_PRIORITY_BOTH)
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
			status = mb_malloc(verbose,n_priority_angle*sizeof(double),
				&priority_angle_angle,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,n_priority_angle*sizeof(double),
				&priority_angle_priority,&error);
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

	/* output info */
	if (verbose >= 0)
		{
		fprintf(outfp,"\nMBMOSAIC Parameters:\n");
		fprintf(outfp,"List of input files: %s\n",filelist);
		fprintf(outfp,"Output fileroot:     %s\n",fileroot);
		fprintf(outfp,"Input Data Type:     ");
		if (datatype == MBMOSAIC_DATA_AMPLITUDE)
			fprintf(outfp,"Amplitude\n");
		else if (datatype == MBMOSAIC_DATA_SIDESCAN)
			fprintf(outfp,"Sidescan\n");
		else if (datatype == MBMOSAIC_DATA_FLAT_GRAZING)
			fprintf(outfp,"Flat bottom grazing angle\n");
		else if (datatype == MBMOSAIC_DATA_GRAZING)
			fprintf(outfp,"Grazing angle\n");
		else if (datatype == MBMOSAIC_DATA_SLOPE)
			fprintf(outfp,"Bottom slope\n");
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
			fprintf(outfp, "  Pixels prioritized by grazing angle\n");
			fprintf(outfp, "  Pixel prioritization file: %s\n", pfile);
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
		if (! clip) 
			fprintf(outfp,"  Spline interpolation not applied\n");
		if (clip) 
			{
			fprintf(outfp,"  Spline interpolation applied with clipping dimension: %d\n",clip);
			fprintf(outfp,"  Spline tension (range 0.0 to infinity): %f\n",tension);
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
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&grid,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&norm,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&maxpriority,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(int),&cnt,&error);
	if (clip != 0)
	    status = mb_malloc(verbose,gxdim*gydim*sizeof(int),&num,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&sigma,&error);
	status = mb_malloc(verbose,xdim*ydim*sizeof(float),&output,&error);

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

	/***** do first pass gridding *****/
	if (grid_mode == MBMOSAIC_SINGLE_BEST
	    || priority_mode != MBMOSAIC_PRIORITY_NONE)
	{

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
		mb_memory_clear(verbose, &error);
		exit(error);
		}
	while ((status = mb_datalist_read(verbose,datalist,
			file,&format,&file_weight,&error))
			== MB_SUCCESS)
		{
		ndatafile = 0;

		/* if format > 0 then input is multibeam file */
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

		/* initialize the multibeam file */
		if (file_in_bounds == MB_YES)
		    {
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
		    if (beams_amp > pixels_ss)
		    	{
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, 
							    sizeof(double), (void **)&angles, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, 
							    sizeof(double), (void **)&priorities, &error);
			}
		    else
		    	{
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
							    sizeof(double), (void **)&angles, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
							    sizeof(double), (void **)&priorities, &error);
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
			status = mb_get(verbose,mbio_ptr,&kind,
				&rpings,time_i,&time_d,
				&navlon,&navlat,
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

			/* get factors for lon lat calculations */
			if (error == MB_ERROR_NO_ERROR)
				{
				mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
				headingx = sin(DTR*heading);
				headingy = cos(DTR*heading);
				}

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
				}
			    }

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
			    
			  /* get angles and priorities */
			  mbmosaic_get_priorities(verbose, priority_mode, file_weight,  
				n_priority_angle, priority_angle_angle, priority_angle_priority, 
				priority_azimuth, priority_azimuth_factor, 
				beams_bath, beamflag, bath, bathacrosstrack,
				work1, work2,  
				bath_default, heading, 
				beams_amp, amp, bathacrosstrack, 
				angles, priorities, &error);

			  /* get bathymetry slopes if needed */
			  if (use_slope == MB_YES)
			    	set_bathyslope(verbose, 
					beams_bath,beamflag,bath,bathacrosstrack,
					&ndepths,depths,depthacrosstrack,
					&nslopes,slopes,slopeacrosstrack,
					&error);

			  /* deal with data */
			  for (ib=0;ib<beams_amp;ib++) 
			    if (mb_beam_ok(beamflag[ib]))
			      {
			      /* get position in grid */
			      ix = (bathlon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] + 0.5*dy)/dy;

			      /* process if in region of interest */
			      if (ix >= 0 
				&& ix < gxdim 
				&& iy >= 0 
				&& iy < gydim)
			        {
			        ix1 = MAX(ix - xtradim, 0);
			        ix2 = MIN(ix + xtradim, gxdim - 1);
			        iy1 = MAX(iy - xtradim, 0);
			        iy2 = MIN(iy + xtradim, gydim - 1);
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				    {
				    /* set grid if highest weight */
				    kgrid = ii*gydim + jj;
				    if (priorities[ib] > maxpriority[kgrid])
					{
					if (use_slope)
						status = get_bathyslope(verbose,
						    ndepths,depths,depthacrosstrack,
						    nslopes,slopes,slopeacrosstrack,
						    depthacrosstrack[ib],
						    &depth,&slope,&error);

					if (datatype == MBMOSAIC_DATA_AMPLITUDE)
					    grid[kgrid] = amp[ib];
					else if (datatype == MBMOSAIC_DATA_FLAT_GRAZING)
					  {
					    if (angles[ib] > 0)
					      grid[kgrid] = angles[ib];
					    else
					      grid[kgrid] = - angles[ib];
					  }
					else if (datatype == MBMOSAIC_DATA_GRAZING)
					  {
					    slope +=  angles[ib];
					    if (slope < 0)
					      slope = - slope;
					    grid[kgrid] = slope;
					  }
					else if (datatype == MBMOSAIC_DATA_SLOPE)
					  {
					    if (slope < 0)
					      slope = - slope;
					    grid[kgrid] = slope;
					  }

					cnt[kgrid] = 1;
					maxpriority[kgrid] = priorities[ib];
					}
				    ndata++;
				    ndatafile++;
				    }
				  }
			      }
			  }
			else if (datatype == MBMOSAIC_DATA_SIDESCAN
				&& error == MB_ERROR_NO_ERROR)
			  {
				
			  /* translate pixel locations to lon/lat */
			  for (ib=0;ib<pixels_ss;ib++)
			    {
			    if (ss[ib] > 0.0)
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
				}
			    }

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
			    
			  /* get angles and priorities */
			  mbmosaic_get_priorities(verbose, priority_mode, file_weight, 
				n_priority_angle, priority_angle_angle, priority_angle_priority, 
				priority_azimuth, priority_azimuth_factor, 
				beams_bath, beamflag, bath, bathacrosstrack,
				work1, work2,  
				bath_default, heading, 
				pixels_ss, ss, ssacrosstrack, 
				angles, priorities, &error);

			  /* deal with data */
			  for (ib=0;ib<pixels_ss;ib++) 
			    if (ss[ib] > 0.0)
			      {
			      /* get position in grid */
			      ix = (sslon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (sslat[ib] - wbnd[2] + 0.5*dy)/dy;
			      
			      /* process if in region of interest */
			      if (ix >= 0 
				&& ix < gxdim 
				&& iy >= 0 
				&& iy < gydim )
			        {
			        ix1 = MAX(ix - xtradim, 0);
			        ix2 = MIN(ix + xtradim, gxdim - 1);
			        iy1 = MAX(iy - xtradim, 0);
			        iy2 = MIN(iy + xtradim, gydim - 1);
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				    {
				    /* set grid if highest weight */
				    kgrid = ii*gydim + jj;
				    if (priorities[ib] > maxpriority[kgrid])
					{
					grid[kgrid] = ss[ib];
					cnt[kgrid] = 1;
					maxpriority[kgrid] = priorities[ib];
					}
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
			fprintf(outfp,"%u data points processed in %s\n",
				ndatafile,file);
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
	if (status = mb_datalist_open(verbose,&datalist,
					filelist,look_processed,&error) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
		}
	while ((status = mb_datalist_read(verbose,datalist,
			file,&format,&file_weight,&error))
			== MB_SUCCESS)
		{
		ndatafile = 0;

		/* if format > 0 then input is multibeam file */
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

		/* initialize the multibeam file */
		if (file_in_bounds == MB_YES)
		    {
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
		    if (beams_amp > pixels_ss)
		    	{
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, 
							    sizeof(double), (void **)&angles, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, 
							    sizeof(double), (void **)&priorities, &error);
			}
		    else
		    	{
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
							    sizeof(double), (void **)&angles, &error);
		    	if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
							    sizeof(double), (void **)&priorities, &error);
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
			status = mb_get(verbose,mbio_ptr,&kind,
				&rpings,time_i,&time_d,
				&navlon,&navlat,
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

			/* get factors for lon lat calculations */
			if (error == MB_ERROR_NO_ERROR)
				{
				mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
				headingx = sin(DTR*heading);
				headingy = cos(DTR*heading);
				}

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
				}
			    }

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
			    
			  /* get angles and priorities */
			  mbmosaic_get_priorities(verbose, priority_mode, file_weight,  
				n_priority_angle, priority_angle_angle, priority_angle_priority, 
				priority_azimuth, priority_azimuth_factor, 
				beams_bath, beamflag, bath, bathacrosstrack,
				work1, work2,  
				bath_default, heading, 
				beams_amp, amp, bathacrosstrack, 
				angles, priorities, &error);

			  /* get bathymetry slopes if needed */
			  if (use_slope == MB_YES)
			    	set_bathyslope(verbose, 
					beams_bath,beamflag,bath,bathacrosstrack,
					&ndepths,depths,depthacrosstrack,
					&nslopes,slopes,slopeacrosstrack,
					&error);

			  /* deal with data */
			  for (ib=0;ib<beams_amp;ib++) 
			    if (mb_beam_ok(beamflag[ib]))
			      {
			      /* get position in grid */
			      ix = (bathlon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] + 0.5*dy)/dy;

			      /* process if in region of interest */
			      if (ix >= 0 
				&& ix < gxdim 
				&& iy >= 0 
				&& iy < gydim)
			        {
			        ix1 = MAX(ix - xtradim, 0);
			        ix2 = MIN(ix + xtradim, gxdim - 1);
			        iy1 = MAX(iy - xtradim, 0);
			        iy2 = MIN(iy + xtradim, gydim - 1);
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				    {
				    /* add to cell if weight high enough */
				    kgrid = ii*gydim + jj;
				    if (priorities[ib] > 0.0 
					&& priorities[ib] >= maxpriority[kgrid] - priority_range)
					{
					if (use_slope)
					    status = get_bathyslope(verbose,
						ndepths,depths,depthacrosstrack,
						nslopes,slopes,slopeacrosstrack,
						depthacrosstrack[ib],
						&depth,&slope,&error);

					xx = wbnd[0] + ii*dx - bathlon[ib];
					yy = wbnd[2] + jj*dy - bathlat[ib];
					norm_weight = file_weight * exp(-(xx*xx + yy*yy)*factor);
					norm[kgrid] += norm_weight;
					if (datatype == MBMOSAIC_DATA_AMPLITUDE)
					  {
					    grid[kgrid] += norm_weight * amp[ib];
					    sigma[kgrid] += norm_weight * amp[ib] * amp[ib];
					  }
					else if (datatype == MBMOSAIC_DATA_FLAT_GRAZING)
					  {
					    if (angles[ib] > 0)
					      grid[kgrid] += norm_weight * angles[ib];
					    else
					      grid[kgrid] -= norm_weight * angles[ib];
					    sigma[kgrid] += norm_weight * angles[ib] * angles[ib];
					  }
					else if (datatype == MBMOSAIC_DATA_GRAZING)
					  {
					    slope += angles[ib];
					    if (slope < 0)
					      slope = - slope;
					    grid[kgrid] += norm_weight * slope;
					    sigma[kgrid] += norm_weight * slope * slope;
					  }
					else if (datatype == MBMOSAIC_DATA_SLOPE)
					  {
					    if (slope < 0)
					      slope = - slope;
					    grid[kgrid] += norm_weight * slope;
					    sigma[kgrid] += norm_weight * slope * slope;
					  }
					if (ii == ix && jj == iy)
					    cnt[kgrid]++;
					}
				    }
				ndata++;
				ndatafile++;
				}
			      }
			  }
			else if (datatype == MBMOSAIC_DATA_SIDESCAN
				&& error == MB_ERROR_NO_ERROR)
			  {
				
			  /* translate pixel locations to lon/lat */
			  for (ib=0;ib<pixels_ss;ib++)
			    {
			    if (ss[ib] > 0.0)
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
				}
			    }

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
			    
			  /* get angles and priorities */
			  mbmosaic_get_priorities(verbose, priority_mode, file_weight,  
				n_priority_angle, priority_angle_angle, priority_angle_priority, 
				priority_azimuth, priority_azimuth_factor, 
				beams_bath, beamflag, bath, bathacrosstrack,
				work1, work2,  
				bath_default, heading, 
				pixels_ss, ss, ssacrosstrack, 
				angles, priorities, &error);

			  /* deal with data */
			  for (ib=0;ib<pixels_ss;ib++) 
			    if (ss[ib] > 0.0)
			      {
			      /* get position in grid */
			      ix = (sslon[ib] - wbnd[0] + 0.5*dx)/dx;
			      iy = (sslat[ib] - wbnd[2] + 0.5*dy)/dy;
			      
			      /* process if in region of interest */
			      if (ix >= 0 
				&& ix < gxdim 
				&& iy >= 0 
				&& iy < gydim )
			        {
			        ix1 = MAX(ix - xtradim, 0);
			        ix2 = MIN(ix + xtradim, gxdim - 1);
			        iy1 = MAX(iy - xtradim, 0);
			        iy2 = MIN(iy + xtradim, gydim - 1);
/*fprintf(stderr," ib:%d ss:%f ix:%d iy:%d priority:%f max:%f range:%f", 
ib,ss[ib],ix,iy,priorities[ib],maxpriority[kgrid],priority_range);*/
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				    {
				    /* add to cell if weight high enough */
				    kgrid = ii*gydim + jj;
				    if (priorities[ib] > 0.0 
					&& priorities[ib] >= maxpriority[kgrid] - priority_range)
					{
					xx = wbnd[0] + ii*dx - sslon[ib];
					yy = wbnd[2] + jj*dy - sslat[ib];
					norm_weight = file_weight * exp(-(xx*xx + yy*yy)*factor);
					grid[kgrid] += norm_weight * ss[ib];
					norm[kgrid] += norm_weight;
					sigma[kgrid] += norm_weight * ss[ib] * ss[ib];
					if (ii == ix && jj == iy)
					    cnt[kgrid]++;
/*fprintf(stderr," kgrid:%d norm_weight:%g grid:%g norm:%g cnt:%d",
kgrid,norm_weight,grid[kgrid],norm[kgrid],cnt[kgrid]);*/
					}
				    }
/*fprintf(stderr,"\n");*/
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
		} /* end if (format > 0) */

		}
	if (datalist != NULL)
		mb_datalist_close(verbose,&datalist,&error);
	if (verbose > 0)
		fprintf(outfp,"\n%u total data points processed in averaging pass\n",ndata);

	}
	/***** end of second pass gridding *****/

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
		xmin = sxmin;
		ymin = symin;
		ddx = dx;
		ddy = dy;
		mb_zgrid(sgrid,&gxdim,&gydim,&xmin,&ymin,
			&ddx,&ddy,sdata,&ndata,
			work1,work2,work3,&cay,&clip);

		/* translate the interpolation into the grid array */
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
	mb_free(verbose,&grid,&error); 
	mb_free(verbose,&norm,&error); 
	mb_free(verbose,&maxpriority,&error); 
	mb_free(verbose,&cnt,&error); 
	if (clip != 0)
	    mb_free(verbose,&num,&error); 
	mb_free(verbose,&sigma,&error); 
	mb_free(verbose,&output,&error); 
	if (n_priority_angle > 0)
		{
		mb_free(verbose,&priority_angle_angle,&error); 
		mb_free(verbose,&priority_angle_priority,&error); 
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
	char	date[MB_PATH_MAXLINE], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
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
		GMT_write_grd(outfile, &grd, a, w, e, s, n, pad, complex);

		/* free memory for output array */
		mb_free(verbose, &a, error);
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
/*
 * function mbmosaic_get_priorities obtains data priorities based on
 * grazing angles and look azimuths
 */
int mbmosaic_get_priorities(
		int	verbose, 
		int	mode, 
		double	file_weight, 
		int	nangle, 
		double	*aangles, 
		double	*apriorities, 
		double	azimuth, 
		double	factor, 
		int	nbath, 
		char	*beamflag, 
		double	*bath, 
		double	*bathacrosstrack, 
		double	*depth, 
		double	*depthacrosstrack, 
		double	bath_default, 
		double	heading, 
		unsigned int	ndata, 
		double	*data, 
		double	*acrosstrack, 
		double	*angles, 
		double	*priorities, 
		int	*error)
{
	char	*function_name = "mbmosaic_get_priorities";
	int	status = MB_SUCCESS;
	int	ndepthgood;
	double	depth_use;
	double	azi_starboard, azi_port;
	double	weight_starboard, weight_port;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       mode:	  %d\n",mode);
		fprintf(stderr,"dbg2       file_weight:	  %f\n",file_weight);
		fprintf(stderr,"dbg2       nangle:        %d\n",nangle);
		fprintf(stderr,"dbg2       grazing angle priorities:\n");
		for (i=0;i<nangle;i++)
			fprintf(stderr,"dbg2       i:%d angle:%f weight:%f\n",
				i, aangles[i], apriorities[i]);
		fprintf(stderr,"dbg2       azimuth:       %f\n",azimuth);
		fprintf(stderr,"dbg2       factor:        %f\n",factor);
		fprintf(stderr,"dbg2       nbath:         %d\n",nbath);
		fprintf(stderr,"dbg2       bathymetry:\n");
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2       i:%d flag:%3d bath:%f xtrack:%f\n",
				i, beamflag[i], bath[i], bathacrosstrack[i]);
		fprintf(stderr,"dbg2       bath_default:  %f\n", bath_default);
		fprintf(stderr,"dbg2       heading:       %f\n", heading);
		fprintf(stderr,"dbg2       amplitude/sidescan data:\n");
		for (i=0;i<ndata;i++)
			fprintf(stderr,"dbg2       i:%d data:%f xtrack:%f\n",
				i, data[i], acrosstrack[i]);
		}
		
	/* initialize priority array */
	if (mode == MBMOSAIC_PRIORITY_NONE)
	    for (i=0;i<ndata;i++)
		{
		priorities[i] = 1.0;
		}
	else
	    for (i=0;i<ndata;i++)
		{
		priorities[i] = 0.0;
		}

	/* get grazing angle priorities */
	if (mode == MBMOSAIC_PRIORITY_ANGLE
		|| mode == MBMOSAIC_PRIORITY_BOTH)
		{
		/* initialize angle array */
		for (i=0;i<ndata;i++)
			{
			angles[i] = 0.0;
			}

		/* initialize depth arrays */
		for (i=0;i<nbath;i++)
		    {
		    depth[i] = 0.0;
		    depthacrosstrack[i] = 0.0;
		    }

		/* fill in array of good depths */
		ndepthgood = 0;
		for (i=0;i<nbath;i++)
		    {
		    if (mb_beam_ok(beamflag[i]))
			{
			depth[ndepthgood] = bath[i];
			depthacrosstrack[ndepthgood] = bathacrosstrack[i];
			/* don't allow duplicate acrosstrack values */
			if (ndepthgood == 0
			    || depthacrosstrack[ndepthgood] 
				> depthacrosstrack[ndepthgood - 1])
			    ndepthgood++;
			}
		    }

		/* now loop over data getting angles */
		for (i=0;i<ndata;i++)
		    {
		    if (data[i] > 0.0)
			{
			if (ndepthgood > 0 
			    && acrosstrack[i] <= depthacrosstrack[0])
			    {
			    angles[i] = RTD *
				    atan(acrosstrack[i] / depth[0]);

			    }
			else if (ndepthgood > 0 
			    && acrosstrack[i] >= depthacrosstrack[ndepthgood-1])
			    {
			    angles[i] = RTD *
				    atan(acrosstrack[i] / depth[ndepthgood-1]);

			    }
			else if (ndepthgood > 1)
			    {
			    for (j=0;j<ndepthgood-1;j++)
				{
				if (acrosstrack[i] >= depthacrosstrack[j]
				    && acrosstrack[i] < depthacrosstrack[j+1])
				    {
				    depth_use = depth[j] 
					+ (depth[j+1] - depth[j])
					* (acrosstrack[i] - depthacrosstrack[j])
					/ (depthacrosstrack[j+1] - depthacrosstrack[j]);
				    angles[i] = RTD *
					    atan(acrosstrack[i] / depth_use);
				    }
				}
			    }
			else if (ndepthgood <= 0)
			    {
			    angles[i] = RTD *
				    atan(acrosstrack[i] / bath_default);

			    }
			
			}
		    }


		/* now loop over data getting angle based priorities */
		for (i=0;i<ndata;i++)
		    {
		    if (data[i] > 0.0)
			{
			if (angles[i] < aangles[0]
			    || angles[i] > aangles[nangle-1])
			    priorities[i] = 0.0;
			else
			    for (j=0;j<nangle-1;j++)
				{
				if (angles[i] >= aangles[j]
				    && angles[i] < aangles[j+1])
				    {
				    priorities[i] = apriorities[j]
					+ (apriorities[j+1] - apriorities[j])
					* (angles[i] - aangles[j])
					/ (aangles[j+1] - aangles[j]);
				    }
				}
			}		    
		    }
		}

	/* get look azimuth priorities */
	if (mode == MBMOSAIC_PRIORITY_AZIMUTH
		|| mode == MBMOSAIC_PRIORITY_BOTH)
		{
		/* get priorities for starboard and port sides of ping */
		azi_starboard = heading - 90.0 - azimuth;
		if (azi_starboard > 180.0)
		    azi_starboard -= 360.0 
			* ((int) ((azi_starboard + 180.0) / 360.0));
		else if (azi_starboard < -180.0)
		    azi_starboard += 360.0 
			* ((int) ((-azi_starboard + 180.0) / 360.0));
		azi_starboard *= factor;
		if (azi_starboard <= -90.0 
		    || azi_starboard >= 90.0)
		    weight_starboard = 0.0;
		else
		    weight_starboard = 
			MAX(cos(DTR * factor * azi_starboard), 0.0);
		azi_port = heading + 90.0 - azimuth;
		if (azi_port > 180.0)
		    azi_port -= 360.0 
			* ((int) ((azi_port + 180.0) / 360.0));
		else if (azi_port < -180.0)
		    azi_port += 360.0 
			* ((int) ((-azi_port + 180.0) / 360.0));
		azi_port *= factor;
		if (azi_port <= -90.0 
		    || azi_port >= 90.0)
		    weight_port = 0.0;
		else
		    weight_port = 
			MAX(cos(DTR * factor * azi_port), 0.0);
		
		/* apply the look azimuth priorities to the data alone */
		if (mode == MBMOSAIC_PRIORITY_AZIMUTH)
		    {
		    for (i=0;i<ndata;i++)
			{
			if (data[i] > 0.0 && acrosstrack[i] < 0.0)
			    priorities[i] = weight_starboard;
			else if (data[i] > 0.0)
			    priorities[i] = weight_port;
			}
		    }
		
		/* apply the look azimuth priorities to the data 
			along with grazing angle priorities */
		else
		    {
		    for (i=0;i<ndata;i++)
			{
			if (data[i] > 0.0 && acrosstrack[i] < 0.0)
			    priorities[i] = weight_starboard * priorities[i];
			else if (data[i] > 0.0)
			    priorities[i] = weight_port * priorities[i];
			}
		    }

		}
		
	/* apply file weighting */
	/* removed 3 Sep 2006 DWC */
/*	for (i=0;i<ndata;i++)
		{
		priorities[i] = file_weight * priorities[i];
		}
*/

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       angles and priorities:\n");
		for (i=0;i<ndata;i++)
			fprintf(stderr,"dbg2       i:%d angle:%f priority:%f\n",
				i, angles[i], priorities[i]);
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
int set_bathyslope(int verbose,
	int nbath, char *beamflag, double *bath, double *bathacrosstrack,
	int *ndepths, double *depths, double *depthacrosstrack, 
	int *nslopes, double *slopes, double *slopeacrosstrack, 
	int *error)
{
	char	*function_name = "set_bathyslope";
	int	status = MB_SUCCESS;
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       nbath:           %d\n",nbath);
		fprintf(stderr,"dbg2       bath:            %d\n",bath);
		fprintf(stderr,"dbg2       bathacrosstrack: %d\n",
			bathacrosstrack);
		fprintf(stderr,"dbg2       bath:\n");
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, bath[i], bathacrosstrack[i]);
		}

	/* first find all depths */
	*ndepths = 0;
	for (i=0;i<nbath;i++)
		{
		if (mb_beam_ok(beamflag[i]))
			{
			depths[*ndepths] = bath[i];
			depthacrosstrack[*ndepths] = bathacrosstrack[i];
			(*ndepths)++;
			}
		}

	/* now calculate slopes */
	*nslopes = *ndepths + 1;
	for (i=0;i<*ndepths-1;i++)
		{
		slopes[i+1] = (depths[i+1] - depths[i])
			/(depthacrosstrack[i+1] - depthacrosstrack[i]);
		slopeacrosstrack[i+1] = 0.5*(depthacrosstrack[i+1] 
			+ depthacrosstrack[i]);
		}
	if (*ndepths > 1)
		{
		slopes[0] = 0.0;
		slopeacrosstrack[0] = depthacrosstrack[0];
		slopes[*ndepths] = 0.0;
		slopeacrosstrack[*ndepths] = 
			depthacrosstrack[*ndepths-1];
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndepths:         %d\n",
			*ndepths);
		fprintf(stderr,"dbg2       depths:\n");
		for (i=0;i<*ndepths;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, depths[i], depthacrosstrack[i]);
		fprintf(stderr,"dbg2       nslopes:         %d\n",
			*nslopes);
		fprintf(stderr,"dbg2       slopes:\n");
		for (i=0;i<*nslopes;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int get_bathyslope(int verbose,
	int ndepths, double *depths, double *depthacrosstrack, 
	int nslopes, double *slopes, double *slopeacrosstrack, 
	double acrosstrack, double *depth,  double *slope, 
	int *error)
{
	char	*function_name = "get_bathyslope";
	int	status = MB_SUCCESS;
	int	found_depth, found_slope;
	int	idepth, islope;
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       ndepths:         %d\n",
			ndepths);
		fprintf(stderr,"dbg2       depths:\n");
		for (i=0;i<ndepths;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, depths[i], depthacrosstrack[i]);
		fprintf(stderr,"dbg2       nslopes:         %d\n",
			nslopes);
		fprintf(stderr,"dbg2       slopes:\n");
		for (i=0;i<nslopes;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr,"dbg2       acrosstrack:     %f\n",acrosstrack);
		}

	/* check if acrosstrack is in defined interval */
	found_depth = MB_NO;
	found_slope = MB_NO;
	if (ndepths > 1)
	if (acrosstrack >= depthacrosstrack[0]
		&& acrosstrack <= depthacrosstrack[ndepths-1])
	    {

	    /* look for depth */
	    idepth = -1;
	    while (found_depth == MB_NO && idepth < ndepths - 2)
		{
		idepth++;
		if (acrosstrack >= depthacrosstrack[idepth]
		    && acrosstrack <= depthacrosstrack[idepth+1])
		    {
		    *depth = depths[idepth] 
			    + (acrosstrack - depthacrosstrack[idepth])
			    /(depthacrosstrack[idepth+1] 
			    - depthacrosstrack[idepth])
			    *(depths[idepth+1] - depths[idepth]);
		    found_depth = MB_YES;
		    *error = MB_ERROR_NO_ERROR;
		    }
		}

	    /* look for slope */
	    islope = -1;
	    while (found_slope == MB_NO && islope < nslopes - 2)
		{
		islope++;
		if (acrosstrack >= slopeacrosstrack[islope]
		    && acrosstrack <= slopeacrosstrack[islope+1])
		    {
		    *slope = slopes[islope] 
			    + (acrosstrack - slopeacrosstrack[islope])
			    /(slopeacrosstrack[islope+1] 
			    - slopeacrosstrack[islope])
			    *(slopes[islope+1] - slopes[islope]);
		    found_slope = MB_YES;
		    *error = MB_ERROR_NO_ERROR;
		    }
		}
	    }

	/* translate slope to degrees */
	if (found_slope == MB_YES)
	    *slope = RTD * atan(*slope);

	if (isnan(*slope))
	    *slope = 90;

	/* check for failure */
	if (found_depth != MB_YES || found_slope != MB_YES)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_OTHER;
		*depth = 0.0;
		*slope = 0.0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       depth:           %f\n",*depth);
		fprintf(stderr,"dbg2       slope:           %f\n",*slope);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
