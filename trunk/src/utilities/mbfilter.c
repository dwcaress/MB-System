/*--------------------------------------------------------------------
 *    The MB-system:	mbfilter.c	1/16/95
 *    $Id$
 *
 *    Copyright (c) 1995-2012 by
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
 * mbfilter applies one or more simple filters to the specified
 * data (sidescan, beam amplitude, and/or bathymetry). The filters
 * include:
 *   a: boxcar mean filter for smoothing
 *   b: gaussian mean filter for smoothing
 *   c: boxcar median filter for smoothing
 *   d: inverse gradient filter for smoothing
 *   e: edge detection filter for contrast enhancement
 *   f: gradient subtraction filter for contrast enhancement
 * These filters are mostly intended for use with sidescan
 * data, and operate on 3x3 or 5x5 value windows with
 * no accommodation for differences in acrosstrack vs
 * alongtrack sampling.
 * The default input and output streams are stdin and stdout.
 *
 * Author:	D. W. Caress
 * Date:	January 16, 1995
 *
 * $Log: mbfilter.c,v $
 * Revision 5.9  2009/03/02 18:54:40  caress
 * Fixed pixel size problems with mbmosaic, resurrected program mbfilter, and also updated copyright dates in several source files.
 *
 * Revision 5.8  2008/09/13 06:08:09  caress
 * Updates to apply suggested patches to segy handling. Also fixes to remove compiler warnings.
 *
 * Revision 5.7  2007/10/08 16:48:07  caress
 * State of the code on 8 October 2007.
 *
 * Revision 5.6  2006/08/09 22:41:27  caress
 * Fixed programs that read or write grids so that they do not use the GMT_begin() function; these programs will now work when GMT is built in the default fashion, when GMT is built in the default fashion, with "advisory file locking" enabled.
 *
 * Revision 5.5  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.4  2005/03/25 04:43:00  caress
 * Standardized the string lengths used for filenames and comment data.
 *
 * Revision 5.3  2003/04/17 21:17:10  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2001/07/20 00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/03/22 21:14:16  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.18  2000/10/28  00:40:19  caress
 * Applied fix from Gordon Keith to calculation of pings to be
 * updated and dumped.
 *
 * Revision 4.17  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.16  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.15  1999/12/28  00:28:40  caress
 * Fixed bug in calculating data records to hold in buffer.
 *
 * Revision 4.14  1999/12/28  00:23:32  caress
 * Fixed bug in calculating data records to hold in buffer.
 *
 * Revision 4.13  1999/02/04  23:55:08  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.12  1998/12/18  19:44:23  caress
 * MB-System version 4.6beta5
 *
 * Revision 4.11  1998/12/17  22:50:20  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.10  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.9  1997/10/03  18:59:04  caress
 * Fixed problem with sort call.
 *
 * Revision 4.8  1997/07/25  14:28:10  caress
 * Version 4.5beta2
 *
 * Revision 4.7  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.7  1997/04/17  15:14:38  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.6  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.5  1995/11/17  22:33:12  caress
 * Fixed bug using bath data when no amp present.
 * Bug fix provided by Dan Scheirer.
 *
 * Revision 4.4  1995/08/17  15:04:52  caress
 * Revision for release 4.3.
 *
 * Revision 4.3  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.2  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.1  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.0  1995/02/14  21:17:15  caress
 * Version 4.2
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
#include "../../include/mb_io.h"
#include "../../include/mbsys_ldeoih.h"

/* mode defines */
#define	MBFILTER_BATH			0
#define	MBFILTER_AMP			1
#define	MBFILTER_SS			2

#define	MBFILTER_HIPASS_NONE		0
#define	MBFILTER_HIPASS_MEAN		1
#define	MBFILTER_HIPASS_GAUSSIAN	2
#define	MBFILTER_HIPASS_MEDIAN		3
#define	MBFILTER_SMOOTH_NONE		0
#define	MBFILTER_SMOOTH_MEAN		1
#define	MBFILTER_SMOOTH_GAUSSIAN	2
#define	MBFILTER_SMOOTH_MEDIAN		3
#define	MBFILTER_SMOOTH_GRADIENT	4
#define	MBFILTER_CONTRAST_NONE		0
#define	MBFILTER_CONTRAST_EDGE		1
#define	MBFILTER_CONTRAST_GRADIENT	2

#define	MBFILTER_A_NONE			0
#define	MBFILTER_A_HIPASS_MEAN		1
#define	MBFILTER_A_HIPASS_GAUSSIAN	2
#define	MBFILTER_A_HIPASS_MEDIAN	3
#define	MBFILTER_A_SMOOTH_MEAN		4
#define	MBFILTER_A_SMOOTH_GAUSSIAN	5
#define	MBFILTER_A_SMOOTH_MEDIAN	6
#define	MBFILTER_A_SMOOTH_GRADIENT	7
#define	MBFILTER_A_CONTRAST_EDGE	8
#define	MBFILTER_A_CONTRAST_GRADIENT	9

/* MBIO buffer size default */
#define	MBFILTER_BUFFER_DEFAULT	2000

/* ping structure definition */
struct mbfilter_ping_struct
	{
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
	char	*beamflag;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*amp;
	char	*pixelflag;
	double	*ss;
	double	*ssacrosstrack;
	double	*ssalongtrack;
	double	*dataprocess;
	double	*datasave;
	int	ndatapts;
	double	*data_i_ptr;
	double	*data_f_ptr;
	char	*flag_ptr;
	};

/* filter structure definition */
#define	MBFILTER_NFILTER_MAX	10
struct mbfilter_filter_struct
	{
	int	mode;
	int	xdim;
	int	ldim;
	int	iteration;
	int	threshold;
	double	threshold_lo;
	double	threshold_hi;
	double	hipass_offset;
	};

/* function prototypes */
int hipass_mean(int verbose, int n, double *val, double *wgt,
		double *hipass, int *error);
int hipass_gaussian(int verbose, int n, double *val, double *wgt, double *dis,
		double *hipass, int *error);
int hipass_median(int verbose, int n, double *val, double *wgt,
		double *hipass, int *error);
int smooth_mean(int verbose, int n, double *val, double *wgt,
		double *smooth, int *error);
int smooth_gaussian(int verbose, int n, double *val, double *wgt, double *dis,
		double *smooth, int *error);
int smooth_median(int verbose, double original,
		int apply_threshold, double threshold_lo, double threshold_hi,
		int n, double *val, double *wgt,
		double *smooth, int *error);
int smooth_gradient(int verbose, int n, double *val, double *wgt,
		double *smooth, int *error);
int contrast_edge(int verbose, int n, double *val, double *grad,
		double *result, int *error);
int contrast_gradient(int verbose, int n, double *val, double *wgt,
		double *result, int *error);
int mbcopy_any_to_mbldeoih(int verbose, int system,
		int kind, int *time_i, double time_d,
		double navlon, double navlat, double speed, double heading,
		double draft, double altitude,
		double roll, double pitch, double heave,
		double	beamwidth_xtrack, double beamwidth_ltrack,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment,
		char *ombio_ptr, char *ostore_ptr,
		int *error);

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "MBFILTER";
	char help_message[] =
"mbfilter applies one or more simple filters to the specified\n\t\
data (sidescan and/or beam amplitude). The filters\n\t\
include:\n\t\
  - boxcar mean for lo-pass filtering (-S1)\n\t\
  - gaussian mean for lo-pass filtering (-S2)\n\t\
  - boxcar median for lo-pass filtering (-S3)\n\t\
  - inverse gradient for lo-pass filtering (-S4)\n\t\
  - boxcar mean subtraction for hi-pass filtering (-D1)\n\t\
  - gaussian mean subtraction for hi-pass filtering (-D2)\n\t\
  - boxcar median subtraction for hi-pass filtering (-D3)\n\t\
  - edge detection for contrast enhancement (-C1)\n\t\
  - gradient magnitude subtraction for contrast enhancement (-C2)\n\t\
These filters are primarily intended for use with sidescan\n\t\
data. In particular, the lo-pass or smoothing filters\n\t\
can be used for first-order speckle reduction in sidescan\n\t\
data, and the hi-pass filters can be used to emphasize\n\t\
fine scale structure in the data.\n\t\
The default input and output streams are stdin and stdout.\n";

	char usage_message[] = "mbfilter [\
-Akind -Byr/mo/da/hr/mn/sc\n\t\
-Cmode/xdim/ldim/iteration\n\t\
-Dmode/xdim/ldim/iteration/offset\n\t\
-Eyr/mo/da/hr/mn/sc -Fformat -Iinfile -Nbuffersize\n\t\
-Rwest/east/south/north -Smode/xdim/ldim/iteration\n\t\
-Tthreshold -V -H]";
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message;

	/* MBIO read control parameters */
	int	read_datalist = MB_NO;
	char	read_file[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	format;
	int	system;
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
	int	obeams_bath;
	int	obeams_amp;
	int	opixels_ss;
	char	file[MB_PATH_MAXLINE];
	void	*imbio_ptr = NULL;

	/* MBIO write control parameters */
	char	ofile[MB_PATH_MAXLINE];
	void	*ombio_ptr = NULL;

	/* mbio read and write values */
	struct mb_io_struct *imb_io_ptr;
	struct mb_io_struct *omb_io_ptr;
	void	*store_ptr;
	int	kind;
	int	ndata = 0;
	char	comment[MB_COMMENT_MAXLINE];

	/* buffer handling parameters */
	int	n_buffer_max = MBFILTER_BUFFER_DEFAULT;
	int	nhold = 0;
	int	nhold_ping = 0;
	int	nload;
	int	nunload;
	int	nread;
	int	nreadtot = 0;
	int	nwrite;
	int	nwritetot = 0;
	int	nexpect;
	struct mbfilter_ping_struct ping[MBFILTER_BUFFER_DEFAULT];
	int	first = MB_YES;
	int	done;

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	/* processing control variables */
	int	datakind = MBFILTER_SS;
	int	num_filters = 0;
	struct mbfilter_filter_struct filters[MBFILTER_NFILTER_MAX];
	int	hipass_mode = MBFILTER_HIPASS_NONE;
	int	hipass_xdim = 10;
	int	hipass_ldim = 3;
	int	hipass_iter = 1;
	double	hipass_offset = 1000.0;
	int	smooth_mode = MBFILTER_SMOOTH_NONE;
	int	smooth_xdim = 3;
	int	smooth_ldim = 3;
	int	smooth_iter = 1;
	int	contrast_mode = MBFILTER_CONTRAST_NONE;
	int	contrast_xdim = 5;
	int	contrast_ldim = 5;
	int	contrast_iter = 1;
	int	apply_threshold = MB_NO;
	double	threshold_lo = 0.0;
	double	threshold_hi = 0.0;
	int	nweight;
	int	nweightmax;
	double	*weights;
	double	*values;
	double	*distances;
	int	iteration;

	int	read_data;
	double	*dataptr0, *dataptr1;
	char	*flagptr0, *flagptr1;
	double	ddis;
	int	ndatapts;
	int	ifilter, ndx, ndl;
	int	ia,  ib;
	int	ja,  jb,  jbeg,  jend;
	int	i, j, ii, jj, n;

	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset all defaults but the format and lonflip */
	pings = 1;
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

	/* set default input and output */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:F:f:HhI:i:N:n:R:r:S:s:T:t:Vv")) != -1)
	  switch (c)
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%d", &datakind);
			if (datakind != MBFILTER_SS && datakind != MBFILTER_AMP)
				datakind = MBFILTER_SS;
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'C':
		case 'c':
			n = sscanf (optarg,"%d/%d/%d/%d",
				&contrast_mode, &contrast_xdim,
				&contrast_ldim, &contrast_iter);
			if (n >= 3)
				{
				filters[num_filters].mode = contrast_mode + 7;
				filters[num_filters].xdim = contrast_xdim;
				filters[num_filters].ldim = contrast_ldim;
				filters[num_filters].threshold = MB_NO;
				}
			if (n >= 4)
				filters[num_filters].iteration = contrast_iter;
			else
				filters[num_filters].iteration = 1;
			if (n >= 3)
				num_filters++;
			flag++;
			break;
		case 'D':
		case 'd':
			n = sscanf (optarg,"%d/%d/%d/%d/%lf",
				&hipass_mode, &hipass_xdim,
				&hipass_ldim, &hipass_iter,
				&hipass_offset);
			if (n >= 3)
				{
				filters[num_filters].mode = hipass_mode;
				filters[num_filters].xdim = hipass_xdim;
				filters[num_filters].ldim = hipass_ldim;
				filters[num_filters].threshold = MB_NO;
				}
			if (n >= 4)
				filters[num_filters].iteration = hipass_iter;
			else
				filters[num_filters].iteration = 1;
			if (n >= 5)
				filters[num_filters].hipass_offset = hipass_offset;
			else
				filters[num_filters].hipass_offset = 1000.0;
			if (n >= 3)
				num_filters++;
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
			etime_i[6] = 0;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d", &n_buffer_max);
			if (n_buffer_max > MBFILTER_BUFFER_DEFAULT
			    || n_buffer_max < 10)
			    n_buffer_max = MBFILTER_BUFFER_DEFAULT;
			flag++;
			break;
		case 'R':
		case 'r':
			mb_get_bounds(optarg, bounds);
			flag++;
			break;
		case 'S':
		case 's':
			n = sscanf (optarg,"%d/%d/%d/%d/%lf/%lf",
				&smooth_mode, &smooth_xdim,
				&smooth_ldim, &smooth_iter,
				&threshold_lo, &threshold_hi);
			if (n >= 3)
				{
				filters[num_filters].mode = smooth_mode + 3;
				filters[num_filters].xdim = smooth_xdim;
				filters[num_filters].ldim = smooth_ldim;
				}
			if (n >= 4)
				filters[num_filters].iteration = smooth_iter;
			else
				filters[num_filters].iteration = 1;
			if (n >= 6)
				{
				filters[num_filters].threshold = MB_YES;
				filters[num_filters].threshold_lo = threshold_lo;
				filters[num_filters].threshold_hi = threshold_hi;
				}
			else if (apply_threshold == MB_YES)
				{
				filters[num_filters].threshold = MB_YES;
				filters[num_filters].threshold_lo = threshold_lo;
				filters[num_filters].threshold_hi = threshold_hi;
				}
			else
				filters[num_filters].threshold = MB_NO;
			if (n >= 3)
				num_filters++;
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf/%lf",
				&threshold_lo, &threshold_hi);
			apply_threshold = MB_YES;
			flag++;
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
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* set data type if not set properly */
	if (datakind != MBFILTER_BATH && datakind != MBFILTER_AMP)
		datakind = MBFILTER_SS;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       help:           %d\n",help);
		fprintf(stderr,"dbg2       pings:          %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:        %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:      %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:      %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:      %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:      %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:     %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:     %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:     %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:     %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:     %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:     %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:     %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:     %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:     %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:     %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:     %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:     %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:     %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:     %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:       %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:        %f\n",timegap);
		fprintf(stderr,"dbg2       data format:    %d\n",format);
		fprintf(stderr,"dbg2       read_file:      %s\n",read_file);
		fprintf(stderr,"dbg2       datakind:       %d\n",datakind);
		fprintf(stderr,"dbg2       n_buffer_max:   %d\n",n_buffer_max);
		fprintf(stderr,"dbg2       num_filters:    %d\n",num_filters);
		for (i=0;i<num_filters;i++)
			{
			fprintf(stderr,"dbg2       filters[%d].mode:          %d\n",i,filters[i].mode);
			fprintf(stderr,"dbg2       filters[%d].xdim:          %d\n",i,filters[i].xdim);
			fprintf(stderr,"dbg2       filters[%d].ldim:          %d\n",i,filters[i].ldim);
			fprintf(stderr,"dbg2       filters[%d].iteration:     %d\n",i,filters[i].iteration);
			fprintf(stderr,"dbg2       filters[%d].threshold:     %d\n",i,filters[i].threshold);
			fprintf(stderr,"dbg2       filters[%d].threshold_lo:  %f\n",i,filters[i].threshold_lo);
			fprintf(stderr,"dbg2       filters[%d].threshold_hi:  %f\n",i,filters[i].threshold_hi);
			fprintf(stderr,"dbg2       filters[%d].hipass_offset: %f\n",i,filters[i].hipass_offset);
			}
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* output some information */
	if (verbose > 0)
		{
		if (datakind == MBFILTER_BATH)
			fprintf(stderr, "\nProcessing bathymetry data...\n");
		else if (datakind == MBFILTER_AMP)
			fprintf(stderr, "\nProcessing beam amplitude data...\n");
		else if (datakind == MBFILTER_SS)
			fprintf(stderr, "\nProcessing sidescan data...\n");
		fprintf(stderr, "Number of filters to be applied: %d\n\n", num_filters);
		for (i=0;i<num_filters;i++)
			{
			if (filters[i].mode == MBFILTER_A_HIPASS_MEAN)
				fprintf(stderr,"Filter %d: High pass mean subtraction\n",i);
			else if (filters[i].mode == MBFILTER_A_HIPASS_GAUSSIAN)
				fprintf(stderr,"Filter %d: High pass Gaussian subtraction\n",i);
			else if (filters[i].mode == MBFILTER_A_HIPASS_MEDIAN)
				fprintf(stderr,"Filter %d: High pass median subtraction\n",i);
			else if (filters[i].mode == MBFILTER_A_SMOOTH_MEAN)
				fprintf(stderr,"Filter %d: Low pass mean\n",i);
			else if (filters[i].mode == MBFILTER_A_SMOOTH_GAUSSIAN)
				fprintf(stderr,"Filter %d: Low pass Gaussian\n",i);
			else if (filters[i].mode == MBFILTER_A_SMOOTH_MEDIAN)
				fprintf(stderr,"Filter %d: Low pass median\n",i);
			else if (filters[i].mode == MBFILTER_A_SMOOTH_GRADIENT)
			 	fprintf(stderr,"Filter %d: Low pass gradient\n",i);
			else if (filters[i].mode == MBFILTER_A_CONTRAST_EDGE)
				fprintf(stderr,"Filter %d: Contrast edge\n",i);
			else if (filters[i].mode == MBFILTER_A_CONTRAST_GRADIENT)
				fprintf(stderr,"Filter %d: Contrast gradient\n",i);
			fprintf(stderr, "          Acrosstrack dimension: %d\n", filters[i].xdim);
			fprintf(stderr, "          Alongtrack dimension:  %d\n", filters[i].ldim);
			fprintf(stderr, "          Iterations:            %d\n", filters[i].iteration);
			if (filters[i].mode == MBFILTER_A_SMOOTH_MEDIAN)
				{
				if (filters[i].threshold == MB_YES)
					{
					fprintf(stderr, "          Threshold applied\n");
					fprintf(stderr, "          Threshold_lo:          %f\n", filters[i].threshold_lo);
					fprintf(stderr, "          Threshold_hi:          %f\n", filters[i].threshold_hi);
					}
				else
					{
					fprintf(stderr, "          Threshold not applied\n");
					}
				}
			if (filters[i].mode >= MBFILTER_A_HIPASS_MEAN
				&& filters[i].mode <= MBFILTER_A_HIPASS_MEDIAN)
				{
				fprintf(stderr, "          Hipass_offset:         %f\n", filters[i].hipass_offset);
				}
			fprintf(stderr, "\n");
			}
		}

	/* open file list */
	if (read_datalist == MB_YES)
	    {
	    if ((status = mb_datalist_open(verbose,&datalist,
					    read_file,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			read_file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if ((status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error))
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(file, read_file);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
	{

	/* check for format with amplitude or sidescan data */
	status = mb_format_system(verbose,&format,&system,&error);
	status = mb_format_dimensions(verbose,&format,
			&beams_bath,&beams_amp,&pixels_ss,&error);
	if (datakind == MBFILTER_BATH
		&& beams_bath <= 0)
		{
		fprintf(stderr,"\nProgram <%s> is in bathymetry mode.\n",program_name);
		fprintf(stderr,"Format %d is unacceptable because it does not include sidescan data.\n",format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_FORMAT;
		exit(error);
		}
	if (datakind == MBFILTER_SS
		&& pixels_ss <= 0)
		{
		fprintf(stderr,"\nProgram <%s> is in sidescan mode.\n",program_name);
		fprintf(stderr,"Format %d is unacceptable because it does not include sidescan data.\n",format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_FORMAT;
		exit(error);
		}
	if (datakind == MBFILTER_AMP
		&& beams_amp <= 0)
		{
		fprintf(stderr,"\nProgram <%s> is in amplitude mode.\n",program_name);
		fprintf(stderr,"Format %d is unacceptable because it does not include amplitude data.\n",format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_FORMAT;
		exit(error);
		}

	/* initialize reading the input swath sonar file */
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	imb_io_ptr = (struct mb_io_struct *) imbio_ptr;

	/* initialize writing the output swath sonar file */
	if (datakind == MBFILTER_BATH)
		sprintf(ofile, "%s.ffb", file);
	else if (datakind == MBFILTER_AMP)
		sprintf(ofile, "%s.ffa", file);
	else if (datakind == MBFILTER_SS)
		sprintf(ofile, "%s.ffs", file);
	if ((status = mb_write_init(
		verbose,ofile,71,&ombio_ptr,
		&obeams_bath,&obeams_amp,&opixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	omb_io_ptr = (struct mb_io_struct *) ombio_ptr;

	/* allocate memory for data arrays */
	for (i=0;i<n_buffer_max;i++)
		{
		ping[i].beamflag = NULL;
		ping[i].bath = NULL;
		ping[i].amp = NULL;
		ping[i].bathacrosstrack = NULL;
		ping[i].bathalongtrack = NULL;
		ping[i].pixelflag = NULL;
		ping[i].ss = NULL;
		ping[i].ssacrosstrack = NULL;
		ping[i].ssalongtrack = NULL;
		ping[i].dataprocess = NULL;
		ping[i].datasave = NULL;
		ping[i].data_i_ptr = NULL;
		ping[i].data_f_ptr = NULL;
		ping[i].flag_ptr = NULL;
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(char), (void **)&ping[i].beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&ping[i].bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							sizeof(double), (void **)&ping[i].amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&ping[i].bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&ping[i].bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(char), (void **)&ping[i].pixelflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ping[i].ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ping[i].ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ping[i].ssalongtrack, &error);
		if (datakind == MBFILTER_BATH)
		    {
		    ndatapts = beams_bath;
		    if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&ping[i].dataprocess, &error);
		    if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&ping[i].datasave, &error);
		    }
		else if (datakind == MBFILTER_AMP)
		    {
		    ndatapts = beams_amp;
		    if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							sizeof(double), (void **)&ping[i].dataprocess, &error);
		    if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							sizeof(double), (void **)&ping[i].datasave, &error);
		    }
		else if (datakind == MBFILTER_SS)
		    {
		    ndatapts = pixels_ss;
		    if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ping[i].dataprocess, &error);
		    if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ping[i].datasave, &error);
		    }
		}

	/* get ideal number of ping records to hold */
	nhold_ping = 1;
	nweightmax = 1;
	for (i=0;i<num_filters;i++)
		{
		nhold_ping = MAX(nhold_ping, filters[i].ldim);
		nweightmax = MAX(nweightmax, filters[i].xdim * filters[i].ldim);
		}

	/* allocate memory for weights */
	if (error == MB_ERROR_NO_ERROR)
		status = mb_mallocd(verbose,__FILE__,__LINE__,nweightmax*sizeof(double),
				(void **)&weights,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_mallocd(verbose,__FILE__,__LINE__,nweightmax*sizeof(double),
				(void **)&values,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_mallocd(verbose,__FILE__,__LINE__,nweightmax*sizeof(double),
				(void **)&distances,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
		exit(error);
		}

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	sprintf(comment,"Data filtered by program %s",
		program_name);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"Version %s",rcs_id);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"MB-system Version %s",MB_VERSION);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	strncpy(date,"\0",25);
	right_now = time((time_t *)0);
	strncpy(date,ctime(&right_now),24);
	if ((user_ptr = getenv("USER")) == NULL)
		user_ptr = getenv("LOGNAME");
	if (user_ptr != NULL)
		strcpy(user,user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host,MB_PATH_MAXLINE);
	sprintf(comment,"Run by user <%s> on cpu <%s> at <%s>",
		user,host,date);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (datakind == MBFILTER_BATH)
		{
		sprintf(comment, "Processing bathymetry data...");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else if (datakind == MBFILTER_AMP)
		{
		sprintf(comment, "Processing beam amplitude data...");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else if (datakind == MBFILTER_SS)
		{
		sprintf(comment, "Processing sidescan data...");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	if (hipass_mode == MBFILTER_HIPASS_MEAN)
		{
		sprintf(comment, "applying mean subtraction filter for hipass");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else if (hipass_mode == MBFILTER_HIPASS_GAUSSIAN)
		{
		sprintf(comment, "applying gaussian mean subtraction filter for hipass");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else if (hipass_mode == MBFILTER_HIPASS_MEDIAN)
		{
		sprintf(comment, "applying median subtraction filter for hipass");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	if (hipass_mode != MBFILTER_HIPASS_NONE)
		{
		sprintf(comment, "  filter acrosstrack dimension: %d", hipass_xdim);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment, "  filter alongtrack dimension:  %d", hipass_ldim);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment, "  filter iterations:            %d", hipass_iter);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment, "  filter offset:                %f", hipass_offset);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	if (smooth_mode == MBFILTER_SMOOTH_MEAN)
		{
		sprintf(comment, "applying mean filter for smoothing");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else if (smooth_mode == MBFILTER_SMOOTH_GAUSSIAN)
		{
		sprintf(comment, "applying gaussian mean filter for smoothing");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else if (smooth_mode == MBFILTER_SMOOTH_MEDIAN)
		{
		sprintf(comment, "applying median filter for smoothing");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else if (smooth_mode == MBFILTER_SMOOTH_GRADIENT)
		{
		sprintf(comment, "applying inverse gradient filter for smoothing");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	if (smooth_mode == MBFILTER_SMOOTH_MEDIAN
		&& apply_threshold == MB_YES)
		{
		sprintf(comment, "  filter low ratio threshold:   %f", threshold_lo);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment, "  filter high ratio threshold:  %f", threshold_hi);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	if (smooth_mode != MBFILTER_SMOOTH_NONE)
		{
		sprintf(comment, "  filter acrosstrack dimension: %d", smooth_xdim);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment, "  filter alongtrack dimension:  %d", smooth_ldim);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment, "  filter iterations:            %d", smooth_iter);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	if (contrast_mode == MBFILTER_CONTRAST_EDGE)
		{
		sprintf(comment, "applying edge detection filter for contrast enhancement");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else if (contrast_mode == MBFILTER_CONTRAST_GRADIENT)
		{
		sprintf(comment, "applying gradient subtraction filter for contrast enhancement");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	if (contrast_mode != MBFILTER_CONTRAST_NONE)
		{
		sprintf(comment, "  filter acrosstrack dimension: %d", contrast_xdim);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment, "  filter alongtrack dimension:  %d", contrast_ldim);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment, "  filter iterations:            %d", contrast_iter);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	strncpy(comment,"\0",256);
	sprintf(comment,"Control Parameters:");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"  MBIO data format:   %d",format);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"  Input file:         %s",file);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"  Output file:        %s",ofile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"  Longitude flip:     %d",lonflip);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"  Data kind:         %d",datakind);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment," ");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);

	/* read and write */
	done = MB_NO;
	first = MB_YES;
	ndata = 0;
	nhold = 0;
	nread = 0;
	nwrite = 0;
	while (!done)
		{
		/* load some data into the buffer */
		error = MB_ERROR_NO_ERROR;
		nload = 0;
		nunload = 0;
		nexpect = n_buffer_max - ndata;
		while (status == MB_SUCCESS
			&& ndata < n_buffer_max)
			{
			status = mb_get_all(verbose,imbio_ptr,&store_ptr,&kind,
						ping[ndata].time_i,
						&ping[ndata].time_d,
						&ping[ndata].navlon,
						&ping[ndata].navlat,
						&ping[ndata].speed,
						&ping[ndata].heading,
						&ping[ndata].distance,
						&ping[ndata].altitude,
						&ping[ndata].sonardepth,
						&ping[ndata].beams_bath,
						&ping[ndata].beams_amp,
						&ping[ndata].pixels_ss,
						ping[ndata].beamflag,
						ping[ndata].bath,
						ping[ndata].amp,
						ping[ndata].bathacrosstrack,
						ping[ndata].bathalongtrack,
						ping[ndata].ss,
						ping[ndata].ssacrosstrack,
						ping[ndata].ssalongtrack,
						comment,&error);
			if (status == MB_SUCCESS && kind == MB_DATA_DATA)
				{
				if (datakind == MBFILTER_SS)
					{
					for(i=0;i<ping[ndata].pixels_ss;i++)
						{
						if (ping[ndata].ss[i] > MB_SIDESCAN_NULL)
							ping[ndata].pixelflag[i] = MB_FLAG_NONE;
						else
							ping[ndata].pixelflag[i] = MB_FLAG_NULL;
						}
					}
				status = mb_extract_nav(verbose, imbio_ptr, store_ptr, &kind,
						ping[ndata].time_i,
						&ping[ndata].time_d,
						&ping[ndata].navlon,
						&ping[ndata].navlat,
						&ping[ndata].speed,
						&ping[ndata].heading,
						&ping[ndata].sonardepth,
						&ping[ndata].roll,
						&ping[ndata].pitch,
						&ping[ndata].heave,
						&error);
				status = mb_extract_altitude(verbose, imbio_ptr, store_ptr, &kind,
						&ping[ndata].sonardepth,
						&ping[ndata].altitude,
						&error);
				}
			if (status == MB_SUCCESS && kind == MB_DATA_DATA)
				{
				ndata++;
				nread++;
				nreadtot++;
				nload++;
				}
			if (status == MB_FAILURE && error < 0)
				{
				status = MB_SUCCESS;
				error = MB_ERROR_NO_ERROR;
				}
			}
		if (status == MB_FAILURE && error > 0)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			done = MB_YES;
			}

		/* give the statistics */
		if (verbose >= 1)
			{
			fprintf(stderr,"%d records loaded into buffer\n",nload);
			fprintf(stderr,"%d records held in buffer\n",ndata);
			}

		/* get start of ping output range */
		if (first == MB_YES)
			{
			jbeg = 0;
			first = MB_NO;
			}
		else
			jbeg = MIN(nhold/2+1, ndata);

		/* find number of pings to hold */
		if (done == MB_YES)
			nhold = 0;
		else if (ndata > nhold_ping)
			nhold = nhold_ping;
		else
			nhold = 0;

		/* get end of ping output range */
		if (done == MB_YES)
			jend = ndata - 1;
		else
			{
			jend = ndata - 1 - nhold / 2;
			}
		if (jend < jbeg)
			jend = jbeg;
		if (verbose >= 1)
			{
			fprintf(stderr,"%d survey records being processed\n\n",
				(jend - jbeg + 1));
			}
/*fprintf(stderr, "done:%d jbeg:%d jend:%d ndata:%d nhold_ping:%d nhold:%d\n",
done, jbeg, jend, ndata, nhold_ping, nhold);*/

		/* loop over all filters */
		for (ifilter=0;ifilter<num_filters;ifilter++)
			{
			iteration = 0;
			ndx = filters[ifilter].xdim / 2;
			ndl = filters[ifilter].ldim / 2;

			while (iteration < filters[ifilter].iteration)
				{
				if (verbose > 0)
					fprintf(stderr, "Applying filter %d iteration %d of %d...\n",
							ifilter+1,iteration+1, filters[ifilter].iteration);

				/* set in and out data arrays */
				for (j=0;j<ndata;j++)
				  {
				  if (datakind == MBFILTER_BATH)
				    {
				    ping[j].ndatapts = ping[j].beams_bath;
				    ping[j].data_i_ptr = ping[j].bath;
				    ping[j].flag_ptr = ping[j].beamflag;
				    }
				  else if (datakind == MBFILTER_AMP)
				    {
				    ping[j].ndatapts = ping[j].beams_amp;
				    ping[j].data_i_ptr = ping[j].amp;
				    ping[j].flag_ptr = ping[j].beamflag;
				    }
				  else if (datakind == MBFILTER_SS)
				    {
				    ping[j].ndatapts = ping[j].pixels_ss;
				    ping[j].data_i_ptr = ping[j].ss;
				    ping[j].flag_ptr = ping[j].pixelflag;
				    }
				  ping[j].data_f_ptr = ping[j].dataprocess;
				  }

				/* loop over all the data */
				for (j=0;j<ndata;j++)
				  {

				  /* get beginning and end pings */
				  ja = j - ndl;
				  jb = j + ndl;
				  if (ja < 0) ja = 0;
				  if (jb >= ndata) jb = ndata - 1;

				  /* get data arrays and sizes to be used */
				  dataptr0 = ping[j].data_i_ptr;
				  flagptr0 = ping[j].flag_ptr;
				  ndatapts = ping[j].ndatapts;

				  /* loop over each value */
				  for (i=0;i<ndatapts;i++)
				    {
				    /* get beginning and end values */
				    ia = i - ndx;
				    ib = i + ndx;
				    if (ia < 0) ia = 0;
				    if (ib >= ndatapts) ib = ndatapts - 1;
				    nweight = 0;

				    /* construct arrays of values and weights */
				    if (mb_beam_ok(flagptr0[i]))
				      {
				      /* use primary value if valid */
				      nweight = 1;
				      values[0] = dataptr0[i];
				      distances[0] = 0.0;

				      /* loop over surrounding pings and values */
				      for (jj=ja;jj<=jb;jj++)
		        		for (ii=ia;ii<=ib;ii++)
		        		  {
					  dataptr1 = ping[jj].data_i_ptr;
					  flagptr1 = ping[jj].flag_ptr;
					  if ((jj != j || ii != i)
					    && mb_beam_ok(flagptr1[ii]))
					    {
					    values[nweight] = dataptr1[ii];
					    ddis = 0.0;
					    if (ndx > 0)
						ddis += (ii - i)*(ii - i)/(ndx*ndx);
					    if (ndl > 0)
						ddis += (jj - j)*(jj - j)/(ndl*ndl);
					    distances[nweight] = sqrt(ddis);
					    nweight++;
					    }
					  }
				      }

				    /* get filtered value */
				    if (nweight > 0)
				      {
				      if (filters[ifilter].mode == MBFILTER_A_HIPASS_MEAN)
		        		hipass_mean(verbose, nweight, values,
						weights,
						&ping[j].data_f_ptr[i], &error);
				      else if (filters[ifilter].mode == MBFILTER_A_HIPASS_GAUSSIAN)
		        		hipass_gaussian(verbose, nweight, values,
						weights, distances,
						&ping[j].data_f_ptr[i], &error);
				      else if (filters[ifilter].mode == MBFILTER_A_HIPASS_MEDIAN)
		        		hipass_median(verbose, nweight, values,
						weights,
						&ping[j].data_f_ptr[i], &error);
				      else if (filters[ifilter].mode == MBFILTER_A_SMOOTH_MEAN)
		        		smooth_mean(verbose, nweight, values, weights,
						&ping[j].data_f_ptr[i], &error);
				      else if (filters[ifilter].mode == MBFILTER_A_SMOOTH_GAUSSIAN)
		        		smooth_gaussian(verbose, nweight, values,
						weights, distances,
						&ping[j].data_f_ptr[i], &error);
				      else if (filters[ifilter].mode == MBFILTER_A_SMOOTH_MEDIAN)
		        		smooth_median(verbose, dataptr0[i],
						filters[ifilter].threshold,
						filters[ifilter].threshold_lo,
						filters[ifilter].threshold_hi,
						nweight, values, weights,
						&ping[j].data_f_ptr[i], &error);
				      else if (filters[ifilter].mode == MBFILTER_A_SMOOTH_GRADIENT)
		        		smooth_gradient(verbose, nweight, values, weights,
						&ping[j].data_f_ptr[i], &error);
				      else if (filters[ifilter].mode == MBFILTER_A_CONTRAST_EDGE)
		        		contrast_edge(verbose, nweight, values, weights,
						&ping[j].data_f_ptr[i], &error);
				      else if (filters[ifilter].mode == MBFILTER_A_CONTRAST_GRADIENT)
		        		contrast_gradient(verbose, nweight, values, weights,
						&ping[j].data_f_ptr[i], &error);
				      }
				    else
				      {
				      ping[j].data_f_ptr[i] = MB_SIDESCAN_NULL;
				      }
				    }

				  /* print out progress */
		/*		  if (verbose > 0)
				    fprintf(stderr, "done with ping %d of %d\n",
					(j-jbeg+1), (jend-jbeg+1));*/
				  }

				/* reset initial array and add offset
				    if done with final iteration */
				if (iteration == filters[ifilter].iteration-1)
				  for (j=0;j<ndata;j++)
				    for (i=0;i<ping[j].ndatapts;i++)
					ping[j].data_i_ptr[i] = ping[j].data_f_ptr[i]
						+ filters[ifilter].hipass_offset;
				else
				  for (j=0;j<ndata;j++)
				    for (i=0;i<ping[j].ndatapts;i++)
					ping[j].data_i_ptr[i] = ping[j].data_f_ptr[i];

				/* save results if done with final iteration */
				if (ndata > 0 && iteration == filters[ifilter].iteration-1)
				  {
				  for (j=jbeg;j<=jend;j++)
				    for (i=0;i<ping[j].ndatapts;i++)
					ping[j].datasave[i] = ping[j].data_i_ptr[i];
				  }

				iteration++;
				}
			}

		/* output pings to be cleared from buffer */
		if (ndata > 0)
		for (j=jbeg;j<=jend;j++)
		  {
		  if (datakind == MBFILTER_BATH)
			{
			status = mbcopy_any_to_mbldeoih(verbose, system,
				MB_DATA_DATA, ping[j].time_i, ping[j].time_d,
				ping[j].navlon, ping[j].navlat,
				ping[j].speed, ping[j].heading,
				ping[j].sonardepth, ping[j].altitude,
				ping[j].roll, ping[j].pitch, ping[j].heave,
				imb_io_ptr->beamwidth_xtrack,
				imb_io_ptr->beamwidth_ltrack,
				ping[j].beams_bath,
				0,
				0,
				ping[j].beamflag,
				ping[j].datasave,
				ping[j].amp,
				ping[j].bathacrosstrack,
				ping[j].bathalongtrack,
				ping[j].ss,
				ping[j].ssacrosstrack,
				ping[j].ssalongtrack,
				comment,
				ombio_ptr, omb_io_ptr->store_data, &error);
			}
		  else if (datakind == MBFILTER_AMP)
			{
			status = mbcopy_any_to_mbldeoih(verbose, system,
				MB_DATA_DATA, ping[j].time_i, ping[j].time_d,
				ping[j].navlon, ping[j].navlat,
				ping[j].speed, ping[j].heading,
				ping[j].sonardepth, ping[j].altitude,
				ping[j].roll, ping[j].pitch, ping[j].heave,
				imb_io_ptr->beamwidth_xtrack,
				imb_io_ptr->beamwidth_ltrack,
				ping[j].beams_bath,
				ping[j].beams_amp,
				0,
				ping[j].beamflag,
				ping[j].bath,
				ping[j].datasave,
				ping[j].bathacrosstrack,
				ping[j].bathalongtrack,
				ping[j].ss,
				ping[j].ssacrosstrack,
				ping[j].ssalongtrack,
				comment,
				ombio_ptr, omb_io_ptr->store_data, &error);
			}

		  else if (datakind == MBFILTER_SS)
			{
			status = mbcopy_any_to_mbldeoih(verbose, system,
				MB_DATA_DATA, ping[j].time_i, ping[j].time_d,
				ping[j].navlon, ping[j].navlat,
				ping[j].speed, ping[j].heading,
				ping[j].sonardepth, ping[j].altitude,
				ping[j].roll, ping[j].pitch, ping[j].heave,
				imb_io_ptr->beamwidth_xtrack,
				imb_io_ptr->beamwidth_ltrack,
				ping[j].beams_bath,
				0,
				ping[j].pixels_ss,
				ping[j].beamflag,
				ping[j].bath,
				ping[j].amp,
				ping[j].bathacrosstrack,
				ping[j].bathalongtrack,
				ping[j].datasave,
				ping[j].ssacrosstrack,
				ping[j].ssalongtrack,
				comment,
				ombio_ptr, omb_io_ptr->store_data, &error);
			}

		  /* write the data */
/*fprintf(stderr,"calling mb_write_ping datakind:%d verbose:%d nwrite:%d\n",datakind,verbose,nwrite);*/
		  status = mb_write_ping(verbose,ombio_ptr,omb_io_ptr->store_data,&error);
		  if (status == MB_SUCCESS)
		  	{
		 	nunload++;
		 	nwrite++;
			nwritetot++;
			}
/*verbose = 1;
fprintf(stderr,"nunload:%d nwrite:%d verbose:%d error:%d pointers: %d %d\n",
nunload,nwrite,verbose,error,ombio_ptr,omb_io_ptr->store_data);*/
		  }

		/* save processed data in buffer */
		if (ndata > nhold)
			{
			for (j=0;j<nhold;j++)
			      {
			      jj = ndata - nhold + j;
			      for (i=0;i<7;i++)
		      		ping[j].time_i[i] = ping[jj].time_i[i];
			      ping[j].time_d = ping[jj].time_d;
			      ping[j].navlon = ping[jj].navlon;
			      ping[j].navlat = ping[jj].navlat;
			      ping[j].speed = ping[jj].speed;
			      ping[j].heading = ping[jj].heading;
			      ping[j].distance = ping[jj].distance;
			      ping[j].altitude = ping[jj].altitude;
			      ping[j].sonardepth = ping[jj].sonardepth;
			      ping[j].roll = ping[jj].roll;
			      ping[j].pitch = ping[jj].pitch;
			      ping[j].heave = ping[jj].heave;
			      ping[j].beams_bath = ping[jj].beams_bath;
			      ping[j].beams_amp = ping[jj].beams_amp;
			      ping[j].pixels_ss = ping[jj].pixels_ss;
			      for (i=0;i<ping[j].beams_bath;i++)
		      		{
				ping[j].beamflag[i] = ping[jj].beamflag[i];
				ping[j].bath[i] = ping[jj].bath[i];
				ping[j].bathacrosstrack[i] = ping[jj].bathacrosstrack[i];
				ping[j].bathalongtrack[i] = ping[jj].bathalongtrack[i];
				}
			      for (i=0;i<ping[j].beams_amp;i++)
		      		{
				ping[j].amp[i] = ping[jj].amp[i];
				}
			      for (i=0;i<ping[j].pixels_ss;i++)
		      		{
				ping[j].pixelflag[i] = ping[jj].pixelflag[i];
				ping[j].ss[i] = ping[jj].ss[i];
				ping[j].ssacrosstrack[i] = ping[jj].ssacrosstrack[i];
				ping[j].ssalongtrack[i] = ping[jj].ssalongtrack[i];
				}
			      for (i=0;i<ping[jj].ndatapts;i++)
				ping[j].datasave[i] = ping[jj].datasave[i];
			      }
			ndata = nhold;
			}

		/* give the statistics */
		if (verbose >= 1)
			{
			fprintf(stderr,"\n%d records written from buffer\n",nunload);
			fprintf(stderr,"%d records saved in buffer\n\n",ndata);
			}
		}

	/* close the files */
	status = mb_close(verbose,&imbio_ptr,&error);
	status = mb_close(verbose,&ombio_ptr,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"%d data records read from:  %s\n",nread,file);
		fprintf(stderr,"%d data records written to: %s\n\n",nwrite,ofile);
		}

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if ((status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error))
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
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"%d total data records read\n",nreadtot);
		fprintf(stderr,"%d total data records written\n",nwritetot);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
int hipass_mean(int verbose, int n, double *val, double *wgt,
		double *hipass, int *error)
{
	char	*function_name = "hipass_mean";
	int	status = MB_SUCCESS;
	int	i, nn;


	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %lu\n",(size_t)val);
		fprintf(stderr,"dbg2       wgt:             %lu\n",(size_t)wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get mean */
	*hipass = 0.0;
	nn = 0;
	for (i=0;i<n;i++)
		{
		*hipass += val[i];
		nn++;
		}
	if (nn > 0)
		*hipass = val[0] - *hipass/nn;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       hipass:          %f\n",*hipass);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int hipass_gaussian(int verbose, int n, double *val, double *wgt, double *dis,
		double *hipass, int *error)
{
	char	*function_name = "hipass_gaussian";
	int	status = MB_SUCCESS;
	double	wgtsum;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %lu\n",(size_t)val);
		fprintf(stderr,"dbg2       wgt:             %lu\n",(size_t)wgt);
		fprintf(stderr,"dbg2       dis:             %lu\n",(size_t)dis);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f  dis[%d]: %f\n",
				i, val[i], i, dis[i]);
		}

	/* get weights */
	*hipass = 0.0;
	wgtsum = 0.0;
	for (i=0;i<n;i++)
		{
		wgt[i] = exp(-dis[i]*dis[i]);
		wgtsum += wgt[i];
		}

	if (wgtsum > 0.0)
		{
		/* get value */
		*hipass = 0.0;
		for (i=0;i<n;i++)
			{
			*hipass += wgt[i]*val[i];
			}
		*hipass = val[0] - *hipass/wgtsum;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       hipass:          %f\n",*hipass);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int hipass_median(int verbose, int n, double *val, double *wgt,
		double *hipass, int *error)
{
	char	*function_name = "hipass_median";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %lu\n",(size_t)val);
		fprintf(stderr,"dbg2       wgt:             %lu\n",(size_t)wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* start */
	*hipass = 0.0;

	/* sort values and get median value */
	if (n > 0)
		{
		qsort((char *)val,n,sizeof(double),(void *)mb_double_compare);
		*hipass = val[0] - val[n/2];
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       hipass:          %f\n",*hipass);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int smooth_mean(int verbose, int n, double *val, double *wgt,
		double *smooth, int *error)
{
	char	*function_name = "smooth_mean";
	int	status = MB_SUCCESS;
	int	i, nn;


	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %lu\n",(size_t)val);
		fprintf(stderr,"dbg2       wgt:             %lu\n",(size_t)wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get mean */
	*smooth = 0.0;
	nn = 0;
	for (i=0;i<n;i++)
		{
		*smooth += val[i];
		nn++;
		}
	if (nn > 0)
		*smooth = *smooth/nn;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       smooth:          %f\n",*smooth);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int smooth_gaussian(int verbose, int n, double *val, double *wgt, double *dis,
		double *smooth, int *error)
{
	char	*function_name = "smooth_gaussian";
	int	status = MB_SUCCESS;
	double	wgtsum;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %lu\n",(size_t)val);
		fprintf(stderr,"dbg2       wgt:             %lu\n",(size_t)wgt);
		fprintf(stderr,"dbg2       dis:             %lu\n",(size_t)dis);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f  dis[%d]: %f\n",
				i, val[i], i, dis[i]);
		}

	/* get weights */
	*smooth = 0.0;
	wgtsum = 0.0;
	for (i=0;i<n;i++)
		{
		wgt[i] = exp(-dis[i]*dis[i]);
		wgtsum += wgt[i];
		}

	if (wgtsum > 0.0)
		{
		/* get value */
		*smooth = 0.0;
		for (i=0;i<n;i++)
			{
			*smooth += wgt[i]*val[i];
			}
		*smooth = *smooth/wgtsum;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       smooth:          %f\n",*smooth);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int smooth_median(int verbose, double original,
		int apply_threshold, double threshold_lo, double threshold_hi,
		int n, double *val, double *wgt,
		double *smooth, int *error)
{
	char	*function_name = "smooth_median";
	int	status = MB_SUCCESS;
	double	ratio;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       original:        %f\n",original);
		fprintf(stderr,"dbg2       apply_threshold: %d\n",apply_threshold);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %lu\n",(size_t)val);
		fprintf(stderr,"dbg2       wgt:             %lu\n",(size_t)wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* start */
	*smooth = 0.0;

	/* sort values and get median value */
	if (n > 0)
		{
		qsort((char *)val,n,sizeof(double),(void *)mb_double_compare);
		*smooth = val[n/2];
		}

	/* apply thresholding */
	if (apply_threshold == MB_YES)
		{
		ratio = original/(*smooth);
		if (ratio < threshold_hi
			&& ratio > threshold_lo)
			{
/*fprintf(stderr,"IGNORE MEDIAN FILTER: ratio:%f threshold:%f %f original:%f smooth:%f\n",
ratio, threshold_lo, threshold_hi, original, *smooth);*/
			*smooth = original;
			}
/*else
fprintf(stderr,"** APPLY MEDIAN FILTER: ratio:%f threshold:%f %f original:%f smooth:%f\n",
ratio, threshold_lo, threshold_hi, original, *smooth);*/
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       smooth:          %f\n",*smooth);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int smooth_gradient(int verbose, int n, double *val, double *wgt,
		double *smooth, int *error)
{
	char	*function_name = "smooth_gradient";
	int	status = MB_SUCCESS;
	double	wgtsum;
	double	diff;
	int	i, nn;


	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %lu\n",(size_t)val);
		fprintf(stderr,"dbg2       wgt:             %lu\n",(size_t)wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get weights */
	*smooth = 0.0;
	wgtsum = 0.0;
	nn = 0;
	wgt[0] = 0.5;
	for (i=1;i<n;i++)
		{
		diff = fabs(val[i] - val[0]);
		if (diff < 0.01)
			diff = 0.01;
		wgt[i] = 1.0/diff;
		wgtsum += wgt[i];
		nn++;
		}
	if (nn > 0)
		{
		*smooth = wgt[0]*val[0];
		for (i=1;i<n;i++)
			{
			*smooth += 0.5*wgt[i]*val[i]/wgtsum;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       smooth:          %f\n",*smooth);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int contrast_edge(int verbose, int n, double *val, double *grad,
		double *result, int *error)
{
	char	*function_name = "contrast_edge";
	int	status = MB_SUCCESS;
	double	edge;
	double	gradsum;
	double	contrast;
	int	i, ii;


	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %lu\n",(size_t)val);
		fprintf(stderr,"dbg2       grad:            %lu\n",(size_t)grad);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get gradients */
	gradsum = 0.0;
	edge = 0.0;
	for (i=0;i<n;i++)
	    {
	    grad[i] = 0.0;
	    for (ii=0;ii<n;ii++)
		    {
		    if (val[ii] > 0.0 && i != ii)
			    {
			    grad[i] += (val[ii] - val[i]) * (val[ii] - val[i]);
			    }
		    }
	    gradsum += grad[i];
	    edge += val[i] * grad[i];
	    }
	edge = edge / gradsum;
	contrast = pow((fabs(val[0] - edge) / fabs(val[0] + edge)), 0.75);
	if (val[0] >= edge)
	    *result = edge * (1.0 + contrast) / (1.0 - contrast);
	else
	    *result = edge * (1.0 - contrast) / (1.0 + contrast);
/*fprintf(stderr, "val: %f %f  edge:%f contrast:%f\n",
val[0], *result, edge, contrast);*/

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       result:          %f\n",*result);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int contrast_gradient(int verbose, int n, double *val, double *wgt,
		double *result, int *error)
{
	char	*function_name = "contrast_gradient";
	int	status = MB_SUCCESS;
	double	gradient;
	int	i, nn;


	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %lu\n",(size_t)val);
		fprintf(stderr,"dbg2       wgt:             %lu\n",(size_t)wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get weights */
	*result = 0.0;
	gradient = 0.0;
	nn = 0;
	for (i=1;i<n;i++)
		{
		gradient += (val[i] - val[0]) * (val[i] - val[0]);
		nn++;
		}
	gradient = sqrt(gradient);
	*result = val[0] - 2 * gradient;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       result:          %f\n",*result);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbcopy_any_to_mbldeoih(int verbose, int system,
		int kind, int *time_i, double time_d,
		double navlon, double navlat, double speed, double heading,
		double draft, double altitude,
		double roll, double pitch, double heave,
		double	beamwidth_xtrack, double beamwidth_ltrack,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment,
		char *ombio_ptr, char *ostore_ptr,
		int *error)
{
	char	*function_name = "mbcopy_any_to_mbldeoih";
	int	status = MB_SUCCESS;
	struct mbsys_ldeoih_struct *ostore;
	int	i;

	/* get data structure pointer */
	ostore = (struct mbsys_ldeoih_struct *) ostore_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       ombio_ptr:  %lu\n",(size_t)ombio_ptr);
		fprintf(stderr,"dbg2       ostore_ptr: %lu\n",(size_t)ostore_ptr);
		fprintf(stderr,"dbg2       system:     %d\n",system);
		fprintf(stderr,"dbg2       kind:       %d\n",kind);
		}
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV))
		{
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
		fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
		fprintf(stderr,"dbg2       speed:      %f\n",speed);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		fprintf(stderr,"dbg2       draft:      %f\n",draft);
		fprintf(stderr,"dbg2       altitude:   %f\n",altitude);
		fprintf(stderr,"dbg2       roll:       %f\n",roll);
		fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
		fprintf(stderr,"dbg2       heave:      %f\n",heave);
		fprintf(stderr,"dbg2       beamwidth_xtrack: %f\n",beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack: %f\n",beamwidth_ltrack);
		}
	if (verbose >= 2 && kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg2       nbath:      %d\n",
			nbath);
		if (verbose >= 3)
		 for (i=0;i<nbath;i++)
		  fprintf(stderr,"dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2       namp:       %d\n",namp);
		if (verbose >= 3)
		 for (i=0;i<namp;i++)
		  fprintf(stderr,"dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2        nss:       %d\n",nss);
		if (verbose >= 3)
		 for (i=0;i<nss;i++)
		  fprintf(stderr,"dbg3        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
		}
	if (verbose >= 2 && kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}

	/* copy the data  */
	if (ostore != NULL)
		{
		/* set beam widths */
		ostore->beam_xwidth = beamwidth_xtrack;
		ostore->beam_lwidth = beamwidth_ltrack;
		if (system == MB_SYS_SB2100)
			ostore->ss_type = MB_SIDESCAN_LINEAR;
		else
			ostore->ss_type = MB_SIDESCAN_LOGARITHMIC;
		ostore->kind = kind;

		/* insert data */
		if (kind == MB_DATA_DATA)
		        {
			mb_insert_altitude(verbose, ombio_ptr, (void *)ostore,
					draft, altitude,
					error);
			mb_insert_nav(verbose, ombio_ptr, (void *)ostore,
					time_i, time_d,
					navlon, navlat, speed, heading, draft,
					roll, pitch, heave,
					error);
			}
		status = mb_insert(verbose, ombio_ptr, (void *)ostore,
				kind, time_i, time_d,
				navlon, navlat, speed, heading,
				nbath,namp,nss,
				beamflag,bath,amp,bathacrosstrack,
				bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment, error);

		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> completed\n",
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
