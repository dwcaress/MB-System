/*--------------------------------------------------------------------
 *    The MB-system:	mbgrid.c	5/2/94
 *    $Id: mbgrid.c,v 4.22 1995-05-17 21:51:20 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 1995 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBGRID is an utility used to grid bathymetry, amplitude, or 
 * sidescan data contained in a set of multibeam data files.  
 * This program uses a gaussian weighted average scheme to grid 
 * regions covered by multibeam swaths and then fills in gaps between 
 * the swaths (to the degree specified by the user) using a minimum
 * curvature algorithm.
 *
 * This version reinstates the use of the IGPP/SIO zgrid routine
 * for thin plate spline interpolation. The zgrid code has been
 * translated from Fortran to C. The zgrid algorithm is much
 * faster than the Wessel and Smith minimum curvature algorithm
 * from the GMT program surface used in recent versions of mbgrid.
 *
 * Author:	D. W. Caress
 * Date:	February 22, 1993
 * Rewrite:	May 2, 1994
 * Rerewrite:	April 25, 1995
 *
 * $Log: not supported by cvs2svn $
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

/* GMT grd include file */
#include <grd.h>

/* Includes for System 5 type operating system */
#if defined (IRIX) || defined (LYNX)
#include <time.h>
#include <stdlib.h>
#endif

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"

/* gridding algorithms */
#define	MBGRID_WEIGHTED_MEAN	1
#define	MBGRID_MEDIAN_FILTER	2

/* grid format definitions */
#define	MBGRID_ASCII	1
#define	MBGRID_OLDGRD	2
#define	MBGRID_CDFGRD	3

/* gridded data type */
#define	MBGRID_DATA_BATHYMETRY	1
#define	MBGRID_DATA_TOPOGRAPHY	2
#define	MBGRID_DATA_AMPLITUDE	3
#define	MBGRID_DATA_SIDESCAN	4

/* flag for no data in grid */
#define	NO_DATA_FLAG	99999.9
#define	min(A, B)	((A) < (B) ? (A) : (B))
#define	max(A, B)	((A) > (B) ? (A) : (B))

/* compare function for qsort */
int double_compare();

/* program identifiers */
static char rcs_id[] = "$Id: mbgrid.c,v 4.22 1995-05-17 21:51:20 caress Exp $";
static char program_name[] = "MBGRID";
static char help_message[] =  "MBGRID is an utility used to grid bathymetry data contained \nin a set of multibeam data files.  This program uses either a \nGaussian weighted average scheme or a median filter scheme to \ngrid regions covered by multibeam swaths and then can fill in gaps \nbetween the swaths (to the degree specified by the user) using \na minimum curvature interpolation algorithm.";
static char usage_message[] = "mbgrid -Ifilelist -Oroot -Rwest/east/south/north [-Adatatype\n     -Bborder  -Cclip -Dxdim/ydim -Edx/dy -F -Ggridkind -Llonflip -M -N -Ppings -Sspeed\n     -Ttension -Utime -V -Wscale -Xextend]";

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
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
	char	*mbio_ptr = NULL;

	/* mbgrid control variables */
	char	filelist[128];
	char	fileroot[128];
	int	xdim = 0;
	int	ydim = 0;
	int	set_spacing = MB_NO;
	double	dx = 0.0;
	double	dy = 0.0;
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
	double	timediff = 300.0;
	char	ifile[128];
	char	ofile[128];
	char	cfile[128];
	int	filemod = 493;

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
	double	deglontokm, deglattokm;
	double	mtodeglon, mtodeglat;
	double	gbnd[4], wbnd[4];
	double	xlon, ylat, xx, yy;
	double	factor, weight, topofactor;
	int	gxdim, gydim, offx, offy, xtradim;
	double	*grid = NULL;
	double	*gridx = NULL;
	double	*gridy = NULL;
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
	double	plotscale, tick, dlon, contour;
	double	zmin, zmax, zclip;
	int	nmin, nmax;
	double	smin, smax;
	double	dd, d1, d2;
	int	nbinset, nbinzero, nbinspline;

	/* output char strings */
	char	xlabel[128];
	char	ylabel[128];
	char	zlabel[128];
	char	title[128];
	char	nlabel[128];
	char	sdlabel[128];
	char	psviewer[128];
	char	mbproject[128];

	/* old color table - bright standard path through rgb space */
/*	static int ncpt = 5;
 	static int cptr[] = {255, 255, 120,   0,   0};
 	static int cptg[] = {  0, 220, 255, 255,   0};
 	static int cptb[] = {  0,   0,   0, 255, 255};*/

	/* Bill Haxby color table - what a clever guy! */
	/* color table for bathymetry map */
	static int ncptb = 15;
	static int cptbr[] = {
		255, 255, 255, 255, 255,
		240, 205, 138, 106,  87,
		 50,   0,  40,  21,  37 };
	static int cptbg[] = {
		255, 221, 186, 161, 189,
		236, 255, 236, 235, 215,
		190, 160, 127,  92,  57 };
	static int cptbb[] = {
		255, 171, 133,  68,  87,
		121, 162, 174, 255, 255,
		255, 255, 251, 236, 175 };

	/* gray scale color table */
	/* color table for bathymetry map */
	static int ncptg = 9;
	static int cptg[] = {
		255, 223, 191, 159, 127, 
		 95,  63,  31,   0 };

	/* color table for other maps */
	static int ncpto = 13;
	static int cptor[] = { 255,
		 37,  21,  40,   0,  50,
		 87, 106, 138, 205, 240,
		255, 255 };
	static int cptog[] = { 255,
		 57,  92, 127, 160, 190,
		215, 235, 236, 255, 236,
		189, 161 };
	static int cptob[] = { 255,
		175, 236, 251, 255, 255,
		255, 255, 174, 162, 121,
		 87,  68 };

	/* list of standard standard deviation values for color table */
	static double cptsd[] = {
		0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 
		6.0, 7.5, 10.0, 15.0, 20.0, 30.0, 40.0 };

	/* time, user, host variables */
	long int	right_now;
	char	date[25], user[128], host[128];

	/* output stream for basic stuff (stdout if verbose <= 1,
		stderr if verbose > 1) */
	FILE	*outfp;

	/* variables needed to handle Not-a-Number values */
	float	zero = 0.0;
	float	NaN;

	/* other variables */
	FILE	*fp, *dfp;
	int	i, j, k, ii, jj, kk;
	int	kgrid, kout, kint, ib, ix, iy;
	int	ix1, ix2, iy1, iy2;

	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);
	status = mb_env(verbose, psviewer, mbproject);

	/* set default input and output */
	strcpy (filelist, "data.list");
	strcpy (ofile, "stdout");

	/* initialize some values */
	strcpy(fileroot,"grid");
	gbnd[0] = 0.0;
	gbnd[1] = 0.0;
	gbnd[2] = 0.0;
	gbnd[3] = 0.0;
	xdim = 101;
	ydim = 101;
	gxdim = 0;
	gydim = 0;

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:FfG:g:HhI:i:L:l:MmNnO:o:P:p:R:r:S:s:T:t:U:u:VvW:w:X:x:")) != -1)
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
			sscanf (optarg,"%lf/%lf", &dx, &dy);
			set_spacing = MB_YES;
			flag++;
			break;
		case 'F':
		case 'f':
			grid_mode = MBGRID_MEDIAN_FILTER;
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
			sscanf (optarg, "%lf/%lf/%lf/%lf", 
				&gbnd[0],&gbnd[1],&gbnd[2],&gbnd[3]);
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
	if (verbose == 1)
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
		fprintf(outfp,"dbg2       output file root: %s\n",ofile);
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
		fprintf(outfp,"dbg2       psviewer:         %s\n",psviewer);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(outfp,"\n%s\n",help_message);
		fprintf(outfp,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* if bounds not specified then quit */
	if (gbnd[0] >= gbnd[1] || gbnd[2] >= gbnd[3]
		|| gbnd[2] <= -90.0 || gbnd[3] >= 90.0)
		{
		fprintf(outfp,"\nGrid bounds not properly specified:\n\t%f %f %f %f\n",gbnd[0],gbnd[1],gbnd[2],gbnd[3]);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_PARAMETER;
		exit(error);
		}

	/* define NaN in case it's needed */
	if (use_NaN == MB_YES)
		{
		NaN = zero/zero;
		outclipvalue = NaN;
		}

	/* calculate grid properties and other values */
	mb_coor_scale(verbose,0.5*(gbnd[2]+gbnd[3]),&mtodeglon,&mtodeglat);
	deglontokm = 0.001/mtodeglon;
	deglattokm = 0.001/mtodeglat;
	if (set_spacing == MB_YES)
		{
		xdim = (gbnd[1] - gbnd[0])/dx + 1;
		ydim = (gbnd[3] - gbnd[2])/dy + 1;
		gbnd[1] = gbnd[0] + dx*(xdim - 1);
		gbnd[3] = gbnd[2] + dy*(ydim - 1);
		}
	else
		{
		dx = (gbnd[1] - gbnd[0])/(xdim-1);
		dy = (gbnd[3] - gbnd[2])/(ydim-1);
		}
	offx = 0;
	offy = 0;
	if (extend > 0.0)
		{
		offx = (int) (extend*xdim);
		offy = (int) (extend*ydim);
		}
	xtradim = scale + 1;
	gxdim = xdim + 2*offx;
	gydim = ydim + 2*offy;
	wbnd[0] = gbnd[0] - offx*dx;
	wbnd[1] = gbnd[1] + offx*dx;
	wbnd[2] = gbnd[2] - offy*dy;
	wbnd[3] = gbnd[3] + offy*dy;
	bounds[0] = wbnd[0] - (wbnd[1] - wbnd[0]);
	bounds[1] = wbnd[1] + (wbnd[1] - wbnd[0]);
	bounds[2] = wbnd[2] - (wbnd[3] - wbnd[2]);
	bounds[3] = wbnd[3] + (wbnd[3] - wbnd[2]);
	factor = 4.0/(scale*scale*dx*dy*deglontokm*deglattokm);
	if (datatype == MBGRID_DATA_TOPOGRAPHY)
		topofactor = -1.0;
	else
		topofactor = 1.0;

	/* output info */
	if (verbose >= 0)
		{
		fprintf(outfp,"\nMBGRID Parameters:\n");
		fprintf(outfp,"List of input files: %s\n",filelist);
		fprintf(outfp,"Output fileroot:     %s\n",fileroot);
		fprintf(outfp,"Input Data Type:     ");
		if (datatype == MBGRID_DATA_BATHYMETRY)
			fprintf(outfp,"Bathymetry\n");
		else if (datatype == MBGRID_DATA_TOPOGRAPHY)
			fprintf(outfp,"Topography\n");
		else if (datatype == MBGRID_DATA_AMPLITUDE)
			fprintf(outfp,"Amplitude\n");
		else if (datatype == MBGRID_DATA_SIDESCAN)
			fprintf(outfp,"Sidescan\n");
		else
			fprintf(outfp,"Unknown?\n");
		fprintf(outfp,"Gridding algorithm:  ");
		if (grid_mode == MBGRID_MEDIAN_FILTER)
			fprintf(outfp,"Median Filter\n");
		else
			fprintf(outfp,"Gaussian Weighted Mean\n");
		fprintf(outfp,"Grid dimensions: %d %d\n",xdim,ydim);
		fprintf(outfp,"Grid bounds:\n");
		fprintf(outfp,"  Longitude: %9.4f %9.4f\n",gbnd[0],gbnd[1]);
		fprintf(outfp,"  Latitude:  %9.4f %9.4f\n",gbnd[2],gbnd[3]);
		fprintf(outfp,"Working grid dimensions: %d %d\n",gxdim,gydim);
		fprintf(outfp,"Working Grid bounds:\n");
		fprintf(outfp,"  Longitude: %9.4f %9.4f\n",wbnd[0],wbnd[1]);
		fprintf(outfp,"  Latitude:  %9.4f %9.4f\n",wbnd[2],wbnd[3]);
		fprintf(outfp,"Input data bounds:\n");
		fprintf(outfp,"  Longitude: %9.4f %9.4f\n",bounds[0],bounds[1]);
		fprintf(outfp,"  Latitude:  %9.4f %9.4f\n",bounds[2],bounds[3]);
		fprintf(outfp,"Longitude interval: %f degrees or %f km\n",
			dx,dx*deglontokm);
		fprintf(outfp,"Latitude interval:  %f degrees or %f km\n",
			dy,dy*deglattokm);
		if (grid_mode != MBGRID_MEDIAN_FILTER)
			fprintf(outfp,"Gaussian filter 1/e length: %f km\n",
				sqrt((1.0/factor)));
		if (check_time == MB_YES)
			fprintf(outfp,"Swath overlap time threshold: %f minutes\n", 
				timediff/60.);
		if (! clip) 
			fprintf(outfp,"Spline interpolation not applied\n");
		if (clip) 
			{
			fprintf(outfp,"Spline interpolation applied with clipping dimension: %d\n",clip);
			fprintf(outfp,"Spline tension (range 0.0 to 1.0): %f\n",tension);
			}
		if (gridkind == MBGRID_ASCII)
			fprintf(outfp,"Grid format %d:  ascii table\n",gridkind);
		else if (gridkind == MBGRID_CDFGRD)
			fprintf(outfp,"Grid format %d:  GMT version 2 grd (netCDF)\n",gridkind);
		else
			fprintf(outfp,"Grid format %d:  GMT version 1 grd (binary)\n",gridkind);
		if (use_NaN == MB_YES) 
			fprintf(outfp,"NaN values used to flag regions with no data\n");
		else
			fprintf(outfp,"Real value of %f used to flag regions with no data\n",
				NO_DATA_FLAG);
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
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&gridx,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&gridy,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&sigma,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&firsttime,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(int),&cnt,&error);
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

	/***** do weighted mean gridding *****/
	if (grid_mode == MBGRID_WEIGHTED_MEAN)
	{

	/* allocate memory for additional arrays */
	status = mb_malloc(verbose,gxdim*gydim*sizeof(double),&norm,&error);
	status = mb_malloc(verbose,gxdim*gydim*sizeof(int),&num,&error);

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

	/* initialize arrays and calculate gridx and gridy */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
			grid[kgrid] = 0.0;
			xlon = wbnd[0] + i*dx;
			ylat = wbnd[2] + j*dy;
			gridx[kgrid] = deglontokm*(xlon - wbnd[0]);
			gridy[kgrid] = deglattokm*(ylat - wbnd[2]);
			norm[kgrid] = 0.0;
			sigma[kgrid] = 0.0;
			firsttime[kgrid] = 0.0;
			num[kgrid] = 0;
			cnt[kgrid] = 0;
			}

	/* read in data */
	ndata = 0;
	if ((fp = fopen(filelist,"r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	while (fscanf(fp,"%s %d",file,&format) != EOF)
		{

		/* if format > 0 then input is multibeam file */
		if (format > 0)
		{

		/* initialize the multibeam file */
		ndatafile = 0;
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
				&navlon,&navlat,&speed,&heading,&distance,
				&beams_bath,&beams_amp,&pixels_ss,
				bath,amp,bathlon,bathlat,
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
			  for (ib=0;ib<beams_bath;ib++) 
			    if (bath[ib] > 0.0)
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
				    time_ok = MB_NO;
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
			        ix1 = max(ix - xtradim, 0);
			        ix2 = min(ix + xtradim, gxdim - 1);
			        iy1 = max(iy - xtradim, 0);
			        iy2 = min(iy + xtradim, gydim - 1);
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				   {
				   kgrid = ii*gydim + jj;
				   xx = gridx[kgrid] 
					- deglontokm*(bathlon[ib] - wbnd[0]);
				   yy = gridy[kgrid] 
					- deglattokm*(bathlat[ib] - wbnd[2]);
				   weight = exp(-(xx*xx + yy*yy)*factor);
				   norm[kgrid] = norm[kgrid] + weight;
				   grid[kgrid] = grid[kgrid] 
					+ weight*topofactor*bath[ib];
				   sigma[kgrid] = sigma[kgrid] 
					+ weight*bath[ib]*bath[ib];
				   num[kgrid]++;
				   if (ii == ix && jj == iy)
					cnt[kgrid]++;
				   }
				ndata++;
				ndatafile++;
				}
			      }
			  }
			else if (datatype == MBGRID_DATA_AMPLITUDE
				&& error == MB_ERROR_NO_ERROR)
			  {
			  for (ib=0;ib<beams_bath;ib++) 
			    if (amp[ib] > 0.0)
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
				    time_ok = MB_NO;
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
			        ix1 = max(ix - xtradim, 0);
			        ix2 = min(ix + xtradim, gxdim - 1);
			        iy1 = max(iy - xtradim, 0);
			        iy2 = min(iy + xtradim, gydim - 1);
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				   {
				   kgrid = ii*gydim + jj;
				   xx = gridx[kgrid] 
					- deglontokm*(bathlon[ib] - wbnd[0]);
				   yy = gridy[kgrid] 
					- deglattokm*(bathlat[ib] - wbnd[2]);
				   weight = exp(-(xx*xx + yy*yy)*factor);
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
			      }
			  }
			else if (datatype == MBGRID_DATA_SIDESCAN
				&& error == MB_ERROR_NO_ERROR)
			  {
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
				    time_ok = MB_NO;
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
			        ix1 = max(ix - xtradim, 0);
			        ix2 = min(ix + xtradim, gxdim - 1);
			        iy1 = max(iy - xtradim, 0);
			        iy2 = min(iy + xtradim, gydim - 1);
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				   {
				   kgrid = ii*gydim + jj;
				   xx = gridx[kgrid] 
					- deglontokm*(sslon[ib] - wbnd[0]);
				   yy = gridy[kgrid] 
					- deglattokm*(sslat[ib] - wbnd[2]);
				   weight = exp(-(xx*xx + yy*yy)*factor);
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
			      }
			  }
			}
		status = mb_close(verbose,&mbio_ptr,&error);
		mb_free(verbose,&bath,&error); 
		mb_free(verbose,&bathlon,&error); 
		mb_free(verbose,&bathlat,&error); 
		mb_free(verbose,&amp,&error); 
		mb_free(verbose,&ss,&error); 
		mb_free(verbose,&sslon,&error); 
		mb_free(verbose,&sslat,&error); 
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		if (verbose >= 2) 
			fprintf(outfp,"\n");
		if (verbose > 0)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,file);
		} /* end if (format > 0) */

		/* if format == 0 then input is lon,lat,values triples file */
		else if (format == 0)
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
		ndatafile = 0;
		while (fscanf(dfp,"%lf %lf %lf",&tlon,&tlat,&tvalue) != EOF)
			{
			if (tvalue > 0.0)
			      {
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
			      if (ix >= -xtradim 
				&& ix < gxdim + xtradim 
				&& iy >= -xtradim 
				&& iy < gydim + xtradim
				&& time_ok == MB_YES)
			        {
			        ix1 = max(ix - xtradim, 0);
			        ix2 = min(ix + xtradim, gxdim - 1);
			        iy1 = max(iy - xtradim, 0);
			        iy2 = min(iy + xtradim, gydim - 1);
			        for (ii=ix1;ii<=ix2;ii++)
			         for (jj=iy1;jj<=iy2;jj++)
				   {
				   kgrid = ii*gydim + jj;
				   xx = gridx[kgrid] 
					- deglontokm*(tlon - wbnd[0]);
				   yy = gridy[kgrid] 
					- deglattokm*(tlat - wbnd[2]);
				   weight = exp(-(xx*xx + yy*yy)*factor);
				   norm[kgrid] = norm[kgrid] + weight;
				   grid[kgrid] = grid[kgrid] 
					+ weight*topofactor*tvalue;
				   sigma[kgrid] = sigma[kgrid] 
					+ weight*tvalue*tvalue;
				   num[kgrid]++;
				   if (ii == ix && jj == iy)
					cnt[kgrid]++;
				   }
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
	fclose(fp);
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
/*			fprintf(outfp,"%d %d %d  %f %f %f   %d %d %f %f\n",
				i,j,kgrid,
				grid[kgrid],gridx[kgrid],gridy[kgrid],
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

	/* initialize arrays and calculate gridx and gridy */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
			grid[kgrid] = 0.0;
			xlon = wbnd[0] + i*dx;
			ylat = wbnd[2] + j*dy;
			gridx[kgrid] = deglontokm*(xlon - wbnd[0]);
			gridy[kgrid] = deglattokm*(ylat - wbnd[2]);
			sigma[kgrid] = 0.0;
			firsttime[kgrid] = 0.0;
			cnt[kgrid] = 0;
			}

	/* count data */
	ndata = 0;
	if ((fp = fopen(filelist,"r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	while (fscanf(fp,"%s %d",file,&format) != EOF)
		{

		/* if format > 0 then input is multibeam file */
		if (format > 0)
		{

		/* initialize the multibeam file */
		ndatafile = 0;
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
				&navlon,&navlat,&speed,&heading,&distance,
				&beams_bath,&beams_amp,&pixels_ss,
				bath,amp,bathlon,bathlat,
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
			  for (ib=0;ib<beams_bath;ib++) 
			    if (bath[ib] > 0.0)
			      {
			      /* get position in grid */
			      ix = (bathlon[ib] - wbnd[0] - 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] - 0.5*dy)/dy;
			      if (ix >= 0 && ix < gxdim 
				&& iy >= 0 && iy < gydim)
			        {
				kgrid = ix*gydim + iy;
				cnt[kgrid]++;
				ndata++;
				ndatafile++;
				}
			      }
			  }
			else if (datatype == MBGRID_DATA_AMPLITUDE
				&& error == MB_ERROR_NO_ERROR)
			  {
			  for (ib=0;ib<beams_bath;ib++) 
			    if (amp[ib] > 0.0)
			      {
			      ix = (bathlon[ib] - wbnd[0] - 0.5*dx)/dx;
			      iy = (bathlat[ib] - wbnd[2] - 0.5*dy)/dy;
			      if (ix >= 0 && ix < gxdim 
				&& iy >= 0 && iy < gydim)
			        {
				kgrid = ix*gydim + iy;
				cnt[kgrid]++;
				ndata++;
				ndatafile++;
				}
			      }
			  }
			else if (datatype == MBGRID_DATA_SIDESCAN
				&& error == MB_ERROR_NO_ERROR)
			  {
			  for (ib=0;ib<pixels_ss;ib++) 
			    if (ss[ib] > 0.0)
			      {
			      ix = (sslon[ib] - wbnd[0] - 0.5*dx)/dx;
			      iy = (sslat[ib] - wbnd[2] - 0.5*dy)/dy;
			      if (ix >= 0 && ix < gxdim 
				&& iy >= 0 && iy < gydim)
			        {
				kgrid = ix*gydim + iy;
				cnt[kgrid]++;
				ndata++;
				ndatafile++;
				}
			      }
			  }
			}
		status = mb_close(verbose,&mbio_ptr,&error);
		mb_free(verbose,&bath,&error); 
		mb_free(verbose,&bathlon,&error); 
		mb_free(verbose,&bathlat,&error); 
		mb_free(verbose,&amp,&error); 
		mb_free(verbose,&ss,&error); 
		mb_free(verbose,&sslon,&error); 
		mb_free(verbose,&sslat,&error); 
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		if (verbose >= 2) 
			fprintf(outfp,"\n");
		if (verbose > 0)
			fprintf(outfp,"%d data points counted in %s\n",
				ndatafile,file);
		} /* end if (format > 0) */

		/* if format == 0 then input is lon,lat,values triples file */
		else if (format == 0)
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
		ndatafile = 0;
		while (fscanf(dfp,"%lf %lf %lf",&tlon,&tlat,&tvalue) != EOF)
			{
			if (tvalue > 0.0)
			      {
			      ix = (tlon - wbnd[0] - 0.5*dx)/dx;
			      iy = (tlat - wbnd[2] - 0.5*dy)/dy;
			      if (ix >= 0 && ix < gxdim 
				&& iy >= 0 && iy < gydim)
			        {
				kgrid = ix*gydim + iy;
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
			fprintf(outfp,"%d data points counted in %s\n",
				ndatafile,file);
		} /* end if (format == 0) */

		}
	fclose(fp);

	/* now allocate space for the data */
	for (i=0;i<gxdim;i++)
		for (j=0;j<gydim;j++)
			{
			kgrid = i*gydim + j;
			if (cnt[kgrid] > 0)
				{
				if ((data[kgrid] = (double *) 
					calloc(cnt[kgrid],sizeof(double))) 
					== NULL)
					error = MB_ERROR_MEMORY_FAIL;
				cnt[kgrid] = 0;
				}
			}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
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

	/* read in data */
	ndata = 0;
	if ((fp = fopen(filelist,"r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	while (fscanf(fp,"%s %d",file,&format) != EOF)
		{

		/* if format > 0 then input is multibeam file */
		if (format > 0)
		{

		/* initialize the multibeam file */
		ndatafile = 0;
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
				&navlon,&navlat,&speed,&heading,&distance,
				&beams_bath,&beams_amp,&pixels_ss,
				bath,amp,bathlon,bathlat,
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
			  for (ib=0;ib<beams_bath;ib++) 
			    if (bath[ib] > 0.0)
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
				    time_ok = MB_NO;
			          else
				    time_ok = MB_YES;
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
			  for (ib=0;ib<beams_bath;ib++) 
			    if (amp[ib] > 0.0)
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
				    time_ok = MB_NO;
			          else
				    time_ok = MB_YES;
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
				    time_ok = MB_NO;
			          else
				    time_ok = MB_YES;
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
		mb_free(verbose,&bath,&error); 
		mb_free(verbose,&bathlon,&error); 
		mb_free(verbose,&bathlat,&error); 
		mb_free(verbose,&amp,&error); 
		mb_free(verbose,&ss,&error); 
		mb_free(verbose,&sslon,&error); 
		mb_free(verbose,&sslat,&error); 
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		if (verbose >= 2) 
			fprintf(outfp,"\n");
		if (verbose > 0)
			fprintf(outfp,"%d data points processed in %s\n",
				ndatafile,file);
		} /* end if (format > 0) */

		/* if format == 0 then input is lon,lat,values triples file */
		else if (format == 0)
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
		ndatafile = 0;
		while (fscanf(dfp,"%lf %lf %lf",&tlon,&tlat,&tvalue) != EOF)
			{
			if (tvalue > 0.0)
			      {
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
	fclose(fp);
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
					double_compare);
				grid[kgrid] = value[cnt[kgrid]/2];
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
				gridx[kgrid],gridy[kgrid],
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
	if (clip > 0)
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
			fprintf(outfp,"\nDoing spline interpolation with %d data points...\n",ndata);
		cay = tension;
		xmin = sxmin;
		ymin = symin;
		ddx = dx;
		ddy = dy;
		zgrid(sgrid,&gxdim,&gydim,&xmin,&ymin,
			&ddx,&ddy,sdata,&ndata,
			work1,work2,work3,&cay,&clip);

		/* translate the interpolation into the grid array */
		zflag = 5.0e34;
		for (i=0;i<gxdim;i++)
			for (j=0;j<gydim;j++)
				{
				kgrid = i*gydim + j;
				kint = i + j*gxdim;
				if (grid[kgrid] == clipvalue 
					&& sgrid[kint] < zflag)
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

	/* get min max of data distribution */
	nmin = 0;
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
	if (datatype == MBGRID_DATA_BATHYMETRY)
		{
		strcpy(xlabel,"Longitude");
		strcpy(ylabel,"Latitude");
		strcpy(zlabel,"Depth (m)");
		strcpy(nlabel,"Number of Data Points");
		strcpy(sdlabel,"Depth Standard Deviation (m)");
		strcpy(title,"Bathymetry Grid");
		}
	else if (datatype == MBGRID_DATA_TOPOGRAPHY)
		{
		strcpy(xlabel,"Longitude");
		strcpy(ylabel,"Latitude");
		strcpy(zlabel,"Topography (m)");
		strcpy(nlabel,"Number of Data Points");
		strcpy(sdlabel,"Topography Standard Deviation (m)");
		strcpy(title,"Topography Grid");
		}
	else if (datatype == MBGRID_DATA_AMPLITUDE)
		{
		strcpy(xlabel,"Longitude");
		strcpy(ylabel,"Latitude");
		strcpy(zlabel,"Amplitude");
		strcpy(nlabel,"Number of Data Points");
		strcpy(sdlabel,"Amplitude Standard Deviation (m)");
		strcpy(title,"Amplitude Grid");
		}
	else if (datatype == MBGRID_DATA_SIDESCAN)
		{
		strcpy(xlabel,"Longitude");
		strcpy(ylabel,"Latitude");
		strcpy(zlabel,"Sidescan");
		strcpy(nlabel,"Number of Data Points");
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
				&& grid[kgrid] == clipvalue)
				output[kout] = outclipvalue;
			}
	if (gridkind == MBGRID_ASCII)
		{
		strcpy(ofile,"asc_");
		strcat(ofile,fileroot);
		status = write_ascii(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],
			dx,dy,&error);
		}
	else if (gridkind == MBGRID_OLDGRD)
		{
		strcpy(ofile,"grd1_");
		strcat(ofile,fileroot);
		status = write_oldgrd(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],dx,dy,&error);
		}
	else if (gridkind == MBGRID_CDFGRD)
		{
		strcpy(ofile,"grd_");
		strcat(ofile,fileroot);
		status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
			gbnd[0],gbnd[1],gbnd[2],gbnd[3],
			zmin,zmax,dx,dy,
			xlabel,ylabel,zlabel,title,&error);
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
				}
		if (gridkind == MBGRID_ASCII)
			{
			strcpy(ofile,"asc_");
			strcat(ofile,fileroot);
			strcat(ofile,"_num");
			status = write_ascii(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBGRID_OLDGRD)
			{
			strcpy(ofile,"grd1_");
			strcat(ofile,fileroot);
			strcat(ofile,"_num");
			status = write_oldgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBGRID_CDFGRD)
			{
			strcpy(ofile,"grd_");
			strcat(ofile,fileroot);
			strcat(ofile,"_num");
			status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				zmin,zmax,dx,dy,
				xlabel,ylabel,nlabel,title,&error);
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
				}
		if (gridkind == MBGRID_ASCII)
			{
			strcpy(ofile,"asc_");
			strcat(ofile,fileroot);
			strcat(ofile,"_sd");
			status = write_ascii(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBGRID_OLDGRD)
			{
			strcpy(ofile,"grd1_");
			strcat(ofile,fileroot);
			strcat(ofile,"_sd");
			status = write_oldgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				dx,dy,&error);
			}
		else if (gridkind == MBGRID_CDFGRD)
			{
			strcpy(ofile,"grd_");
			strcat(ofile,fileroot);
			strcat(ofile,"_sd");
			status = write_cdfgrd(verbose,ofile,output,xdim,ydim,
				gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				zmin,zmax,dx,dy,
				xlabel,ylabel,sdlabel,title,&error);
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

	/* write command file */
	if (gridkind == MBGRID_ASCII)
		{
		strcpy(cfile,"asc_");
		strcat(cfile,fileroot);
		strcat(cfile,".cmd");
		if ((fp = fopen(cfile,"w")) == NULL)
			{
			fprintf(outfp,"\nError opening output file: %s\n",
				cfile);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			error = MB_ERROR_OPEN_FAIL;
			exit(error);
			}
		fprintf(fp,"#\nshade $1 <<eot\n");
		fprintf(fp,"clear -1\n");
		fprintf(fp,"Color 0\n");
		fprintf(fp,"size $2\n\n");
		fprintf(fp,"xlab %s\nylab %s\ntitle %s\n",xlabel,ylabel,title);
		fprintf(fp,"cbar\nbarlabel %s\n",zlabel);
		fprintf(fp,"letter 0.1 0.1 0.1 0.1 0.1 0.1\n");
		fprintf(fp,"scolors 0 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0\n");
		fprintf(fp,"axes %f %f %f %f 4 2\n",gbnd[0],gbnd[1],gbnd[2],gbnd[3]);
		fprintf(fp,"null 0 11000\n");
		strcpy(ofile,"asc_");
		strcat(ofile,fileroot);
		fprintf(fp,"file %s\nskip 4\nread %d %d\nskip 0\n",ofile,xdim,ydim);
		fprintf(fp,"plot 0\nwait\n");
		fprintf(fp,"output ras_%s\n",ofile);
		fprintf(fp,"stop\neot\n\n");

		if (more == MB_YES)
			{
			fprintf(fp,"#\nshade $1 <<eot\n");
			fprintf(fp,"clear -1\n");
			fprintf(fp,"Color 0\n");
			fprintf(fp,"size $2\n\n");
			fprintf(fp,"xlab %s\nylab %s\ntitle %s\n",
				xlabel,ylabel,title);
			fprintf(fp,"cbar\nbarlabel %s\n",nlabel);
			fprintf(fp,"letter 0.1 0.1 0.1 0.1 0.1 0.1\n");
			fprintf(fp,"axes %f %f %f %f 4 2\n",
				gbnd[0],gbnd[1],gbnd[2],gbnd[3]);
			fprintf(fp,"null 0 9999999\n");
			strcpy(ofile,"asc_");
			strcat(ofile,fileroot);
			strcat(ofile,"_num");
			fprintf(fp,"file %s\nskip 4\nread %d %d\nskip 0\n",
					ofile,xdim,ydim);
			fprintf(fp,"plot 0\nwait\n");
			fprintf(fp,"output ras_%s\n",ofile);
			fprintf(fp,"stop\neot\n\n");

			fprintf(fp,"#\nshade $1 <<eot\n");
			fprintf(fp,"clear -1\n");
			fprintf(fp,"Color 0\n");
			fprintf(fp,"size $2\n\n");
			fprintf(fp,"xlab %s\nylab %s\ntitle %s\n",
				xlabel,ylabel,title);
			fprintf(fp,"cbar\nbarlabel %s\n",sdlabel);
			fprintf(fp,"letter 0.1 0.1 0.1 0.1 0.1 0.1\n");
			fprintf(fp,"axes %f %f %f %f 4 2\n",
				gbnd[0],gbnd[1],gbnd[2],gbnd[3]);
			fprintf(fp,"null 0 999999\n");
			strcpy(ofile,"asc_");
			strcat(ofile,fileroot);
			strcat(ofile,"_sd");
			fprintf(fp,"file %s\nskip 4\nread %d %d\nskip 0\n",
					ofile,xdim,ydim);
			fprintf(fp,"plot 0\nwait\n");
			fprintf(fp,"output ras_%s\n",ofile);
			fprintf(fp,"stop\neot\n\n");
			}
		close(fp);
		chmod(cfile,filemod);
		}
	else if (gridkind == MBGRID_OLDGRD || gridkind == MBGRID_CDFGRD)
		{
		/* open command file */
		strcpy(cfile,"grd_");
		strcat(cfile,fileroot);
		strcat(cfile,".cmd");
		if ((fp = fopen(cfile,"w")) == NULL)
			{
			fprintf(outfp,"\nError opening output file: %s\n",
				cfile);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			error = MB_ERROR_OPEN_FAIL;
			exit(error);
			}

		/* put info header on shellscript */
		fprintf(fp,"#\n# Shellscript to create Postscript color image of gridded data using GMT version 2 utilities\n");
		strncpy(date,"\0",24);
		right_now = time((long *)0);
		strncpy(date,ctime(&right_now),24);
		strcpy(user,getenv("USER"));
		gethostname(host,128);
		fprintf(fp,"# Created by program %s\n# Version %s\n# MB-system Version %s\n# Run by user <%s> on cpu <%s> at <%s>\n",
		program_name,rcs_id,MB_VERSION,user,host,date);

		/* put in conversion of version 1 grd to version 2 grd */
		if (gridkind == MBGRID_OLDGRD)
			{ 
			fprintf(fp,"#\n# Convert version 1 GRD file to version 2\n");
			fprintf(fp,"echo Converting version 1 GRD file to version 2...\n");
			fprintf(fp,"v1tov2 grd1_%s grd_%s\n",fileroot,fileroot);
			}

		/* put meat in shellscript */
		dlon = (gbnd[1] - gbnd[0]);
		plotscale = 6.0/dlon;
		tick = dlon/5;
		if ((zmax - zmin) > 1000.0)
			contour = 100.0;
		else
			contour = 25.0;
		strcpy(ofile,"grd_");
		strcat(ofile,fileroot);
		fprintf(fp,"#\n# Make data map\n");
		fprintf(fp,"echo Making data map...\n");
		fprintf(fp,"#\n# Remove .gmtcommands file...\n");
		fprintf(fp,"rm -f .gmtcommands\n");
		fprintf(fp,"#\n# Make color image\n");
		fprintf(fp,"echo Running grdimage...\n");
		fprintf(fp,"grdimage %s -Jm%1f -C%s.cpt -X1 -Y3 -P -K -V > %s.ps\n",ofile,plotscale,ofile,ofile);
		if (datatype == MBGRID_DATA_TOPOGRAPHY
			|| datatype == MBGRID_DATA_BATHYMETRY)
			{
			fprintf(fp,"#\n# Make contour map\n");
			fprintf(fp,"echo Running grdcontour...\n");
			fprintf(fp,"grdcontour %s -Jm%1f -C%f -L%f/%f -P -O -K -V >> %s.ps\n",
				ofile,plotscale,contour,zmin,zmax,ofile);
			}
		fprintf(fp,"#\n# Make base map\n");
		fprintf(fp,"echo Running psbasemap...\n");
		fprintf(fp,"psbasemap -Jm%1f -R%1f/%1f/%1f/%1f -B%1f/%1f -P -O -K -U -V >> %s.ps\n",plotscale,gbnd[0],gbnd[1],gbnd[2],gbnd[3],tick,tick,ofile);
		fprintf(fp,"#\n# Make color scale\n");
		fprintf(fp,"echo Running psscale...\n");
		fprintf(fp,"psscale -C%s.cpt -D3/-0.5/5/0.15h -B\":.%s:\" -P -O -V >> %s.ps\n",ofile,zlabel,ofile);
		fprintf(fp,"#\n# Show the plot on the screen\n");
		fprintf(fp,"echo Starting %s...\n", psviewer);
		fprintf(fp,"%s %s.ps &\n",psviewer, ofile);
		if (more == MB_YES)
			{
			/* put in conversion of version 1 grd to version 2 grd */
			if (gridkind == MBGRID_OLDGRD)
				{ 
				fprintf(fp,"#\n# Convert version 1 GRD files to version 2\n");
				fprintf(fp,"echo Converting version 1 GRD files to version 2...\n");
				fprintf(fp,"v1tov2 grd1_%s_num grd_%s_num\n",fileroot,fileroot);
				fprintf(fp,"v1tov2 grd1_%s_sd grd_%s_sd\n",fileroot,fileroot);
				}

			strcpy(ofile,"grd_");
			strcat(ofile,fileroot);
			strcat(ofile,"_num");
			fprintf(fp,"#\n# Make data distribution map\n");
			fprintf(fp,"echo Making data distribution map...\n");
			fprintf(fp,"#\n# Remove .gmtcommands file...\n");
			fprintf(fp,"rm -f .gmtcommands\n");
			fprintf(fp,"#\n# Make color image\n");
			fprintf(fp,"echo Running grdimage...\n");
			fprintf(fp,"grdimage %s -Jm%1f -C%s.cpt -X1 -Y3 -P -K -V > %s.ps\n",
				ofile,plotscale,ofile,ofile);
			fprintf(fp,"#\n# Make basemap\n");
			fprintf(fp,"echo Running psbasemap...\n");
			fprintf(fp,"psbasemap -Jm%1f -R%1f/%1f/%1f/%1f -B%1f/%1f -P -O -K -U -V >> %s.ps\n",
				plotscale,gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				tick,tick,ofile);
			fprintf(fp,"#\n# Make color scale\n");
			fprintf(fp,"echo Running psscale...\n");
			fprintf(fp,"psscale -C%s.cpt -D3/-0.5/5/0.15h -B\":.%s:\" -P -O -V >> %s.ps\n",
				ofile,nlabel,ofile);
			fprintf(fp,"#\n# Show the plot on the screen\n");
			fprintf(fp,"echo Starting %s...\n", psviewer);
			fprintf(fp,"%s %s.ps &\n",psviewer, ofile);

			strcpy(ofile,"grd_");
			strcat(ofile,fileroot);
			strcat(ofile,"_sd");
			fprintf(fp,"#\n# Make standard deviation map\n");
			fprintf(fp,"echo Making standard deviation map...\n");
			fprintf(fp,"#\n# Remove .gmtcommands file...\n");
			fprintf(fp,"rm -f .gmtcommands\n");
			fprintf(fp,"#\n# Make color image\n");
			fprintf(fp,"echo Running grdimage...\n");
			fprintf(fp,"grdimage %s -Jm%1f -C%s.cpt -P -X1 -Y3 -K -V > %s.ps\n",
				ofile,plotscale,ofile,ofile);
			fprintf(fp,"#\n# Make basemap\n");
			fprintf(fp,"echo Running psbasemap...\n");
			fprintf(fp,"psbasemap -Jm%1f -R%1f/%1f/%1f/%1f -B%1f/%1f -P -O -K -U -V >> %s.ps\n",
				plotscale,gbnd[0],gbnd[1],gbnd[2],gbnd[3],
				tick,tick,ofile);
			fprintf(fp,"#\n# Make color scale\n");
			fprintf(fp,"echo Running psscale...\n");
			fprintf(fp,"psscale -C%s.cpt -D3/-0.5/5/0.15h -B\":.%s:\" -L -P -O -V >> %s.ps\n",
				ofile,sdlabel,ofile);
			fprintf(fp,"#\n# Show the plot on the screen\n");
			fprintf(fp,"echo Starting %s...\n", psviewer);
			fprintf(fp,"%s %s.ps &\n",psviewer, ofile);
			}
		fprintf(fp,"#\n# All done\n");
		fprintf(fp,"echo All done...\n");

		close(fp);
		chmod(cfile,filemod);

		/* write out color tables */
		strcpy(cfile,"grd_");
		strcat(cfile,fileroot);
		strcat(cfile,".cpt");
		if ((fp = fopen(cfile,"w")) == NULL)
			{
			fprintf(outfp,"\nError opening output file: %s\n",
				cfile);
			fprintf(outfp,"\nProgram <%s> Terminated\n",
				program_name);
			error = MB_ERROR_OPEN_FAIL;
			exit(error);
			}
		if (datatype == MBGRID_DATA_TOPOGRAPHY)
			{
			dd = 1.1*(zmax - zmin)/(ncptb-1);
			for (i=0;i<ncptb-1;i++)
				{
				j = ncptb - 1 - i;
				d1 = zmin - 0.05*(zmax - zmin) + i*dd;
				d2 = zmin - 0.05*(zmax - zmin) + (i+1)*dd;
				fprintf(fp,"%5.0f %d %d %d %5.0f %d %d %d\n",
					d1,cptbr[j],cptbg[j],cptbb[j],
					d2,cptbr[j-1],cptbg[j-1],cptbb[j-1]);
				}
			}
		else if (datatype == MBGRID_DATA_AMPLITUDE
			|| datatype == MBGRID_DATA_SIDESCAN)
			{
			dd = 1.1*(zmax - zmin)/(ncptg-1);
			for (i=0;i<ncptg-1;i++)
				{
				d1 = zmin - 0.05*(zmax - zmin) + i*dd;
				d2 = zmin - 0.05*(zmax - zmin) + (i+1)*dd;
				fprintf(fp,"%5.0f %d %d %d %5.0f %d %d %d\n",
					d1,cptg[i],cptg[i],cptg[i],
					d2,cptg[i+1],cptg[i+1],cptg[i+1]);
				}
			}
		else
			{
			dd = 1.1*(zmax - zmin)/(ncptb-1);
			for (i=0;i<ncptb-1;i++)
				{
				d1 = zmin - 0.05*(zmax - zmin) + i*dd;
				d2 = zmin - 0.05*(zmax - zmin) + (i+1)*dd;
				fprintf(fp,"%5.0f %d %d %d %5.0f %d %d %d\n",
					d1,cptbr[i],cptbg[i],cptbb[i],
					d2,cptbr[i+1],cptbg[i+1],cptbb[i+1]);
				}
			}
		close(fp);

		if (more == MB_YES)
			{
			/* write out color table for data distribution map */
			strcpy(cfile,"grd_");
			strcat(cfile,fileroot);
			strcat(cfile,"_num");
			strcat(cfile,".cpt");
			if ((fp = fopen(cfile,"w")) == NULL)
				{
				fprintf(outfp,"\nError opening output file: %s\n",
					cfile);
				fprintf(outfp,"\nProgram <%s> Terminated\n",
					program_name);
				error = MB_ERROR_OPEN_FAIL;
				exit(error);
				}
			dd = ((double)(max(nmax,ncpto-1)))/(ncpto-1);
			for (i=0;i<ncpto-1;i++)
				{
				d1 = nmin + i*dd;
				d2 = nmin + (i+1)*dd;
				fprintf(fp,"%5.0f %d %d %d %5.0f %d %d %d\n",
					d1,cptor[i],cptog[i],cptob[i],
					d2,cptor[i+1],cptog[i+1],cptob[i+1]);
				}
			close(fp);

			/* write out color table for standard deviation map */
			strcpy(cfile,"grd_");
			strcat(cfile,fileroot);
			strcat(cfile,"_sd");
			strcat(cfile,".cpt");
			if ((fp = fopen(cfile,"w")) == NULL)
				{
				fprintf(outfp,"\nError opening output file: %s\n",
					cfile);
				fprintf(outfp,"\nProgram <%s> Terminated\n",
					program_name);
				error = MB_ERROR_OPEN_FAIL;
				exit(error);
				}
			for (i=0;i<ncpto-2;i++)
				{
				fprintf(fp,"%5.0f %d %d %d %5.0f %d %d %d\n",
					cptsd[i],cptor[i],cptog[i],cptob[i],
					cptsd[i+1],cptor[i+1],cptog[i+1],
					cptob[i+1]);
				}
			if (cptsd[ncpto] > smax)
				fprintf(fp,"%5.0f %d %d %d %5.0f %d %d %d\n",
					cptsd[ncpto-2],cptor[ncpto-2],
					cptog[ncpto-2],cptob[ncpto-2],
					cptsd[ncpto-1],cptor[ncpto-1],
					cptog[ncpto-1],cptob[ncpto-1]);
			else
				fprintf(fp,"%5.0f %d %d %d %5.0f %d %d %d\n",
					cptsd[ncpto-2],cptor[ncpto-2],
					cptog[ncpto-2],cptob[ncpto-2],
					smax+1.0,cptor[ncpto-1],
					cptog[ncpto-1],cptob[ncpto-1]);
			close(fp);
			}

		}

	/* deallocate arrays */
	mb_free(verbose,&grid,&error); 
	mb_free(verbose,&gridy,&error); 
	mb_free(verbose,&gridx,&error); 
	mb_free(verbose,&norm,&error); 
	mb_free(verbose,&num,&error); 
	mb_free(verbose,&cnt,&error); 
	mb_free(verbose,&sigma,&error); 
	mb_free(verbose,&firsttime,&error); 
	mb_free(verbose,&output,&error); 
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
int write_ascii(verbose,outfile,grid,
			nx,ny,xmin,xmax,ymin,ymax,
			dx,dy,error)
int	verbose;
char	*outfile;
float	*grid;
int	nx, ny;
double	xmin, xmax, ymin, ymax, dx, dy;
int	*error;
{
	char	*function_name = "write_ascii";
	int	status = MB_SUCCESS;
	FILE	*fp;
	int	i;
	long int	right_now;
	char	date[25], user[128], host[128];
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
		right_now = time((long *)0);
		strncpy(date,"\0",25);
		strncpy(date,ctime(&right_now),24);
		strcpy(user,getenv("USER"));
		i = gethostname(host,128);
		fprintf(fp,"program run by %s on %s at %s\n",user,host,date);
		fprintf(fp,"%d %d\n%f %f %f %f\n",nx,ny,xmin,xmax,ymin,ymax);
		for (i=0;i<nx*ny;i++)
			{
			fprintf(fp,"%13.5g ",grid[i]);
			if ((i+1) % 6 == 0) fprintf(fp,"\n");
			}
		if ((nx*ny) % 6 != 0) fprintf(fp,"\n");
		close(fp);
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
int write_oldgrd(verbose,outfile,grid,
			nx,ny,xmin,xmax,ymin,ymax,
			dx,dy,error)
int	verbose;
char	*outfile;
float	*grid;
int	nx, ny;
double	xmin, xmax, ymin, ymax, dx, dy;
int	*error;
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
		close(fp);
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
int write_cdfgrd(verbose,outfile,grid,nx,ny,
			xmin,xmax,ymin,ymax,zmin,zmax,dx,dy,
			xlab,ylab,zlab,titl,error)
int	verbose;
char	*outfile;
float	*grid;
int	nx, ny;
double	xmin, xmax, ymin, ymax, zmin, zmax, dx, dy;
char	*xlab, *ylab, *zlab, *titl;
int	*error;
{
	char	*function_name = "write_cdfgrd";
	int	status = MB_SUCCESS;
	struct GRD_HEADER grd;
	float	*a;
	long int	right_now;
	char	date[128], user[128], host[128];
	char	*message;
	int	i, j, kg, ka;
	int	grdstatus;
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
		}

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
	right_now = time((long *)0);
	strncpy(date,"\0",128);
	right_now = time((long *)0);
	strncpy(date,ctime(&right_now),24);
	strcpy(user,getenv("USER"));
	gethostname(host,128);
	sprintf(grd.remark,"This grid created by program %s\nMB-system Version %s\nRun by user <%s> on cpu <%s> at <%s>",
		program_name,MB_VERSION,user,host,date);

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
		write_grd(outfile,&grd,a);

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
int double_compare(a,b)
double	*a;
double	*b;
{
	if (*a > *b)
		return(1);
	else
		return(-1);
}
/*--------------------------------------------------------------------*/
