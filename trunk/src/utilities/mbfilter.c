/*--------------------------------------------------------------------
 *    The MB-system:	mbfilter.c	1/16/95
 *    $Id: mbfilter.c,v 5.4 2005-03-25 04:43:00 caress Exp $
 *
 *    Copyright (c) 1995, 2000, 2003 by
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
 * $Log: not supported by cvs2svn $
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
#include <math.h>
#include <string.h>
#include <time.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"

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

/* MBIO buffer size default */
#define	MBFILTER_BUFFER_DEFAULT	500

/* ping structure definition */
struct mbfilter_ping_struct 
	{
	int	id;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
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

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mbfilter.c,v 5.4 2005-03-25 04:43:00 caress Exp $";
	static char program_name[] = "MBFILTER";
	static char help_message[] =  
"mbfilter applies one or more simple filters to the specified\n\t\
data (sidescan, beam amplitude, and/or bathymetry). The filters\n\t\
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
These filters are mostly intended for use with sidescan\n\t\
data. In particular, the lo-pass or smoothing filters\n\t\
can be used for first-order speckle reduction in sidescan\n\t\
data, and the hi-pass filters can be used to emphasize\n\t\
fine scale structure in the data.\n\t\
The default input and output streams are stdin and stdout.\n";

	static char usage_message[] = "mbfilter [\
-Akind -Byr/mo/da/hr/mn/sc\n\t\
-Cmode/xdim/ldim/iteration\n\t\
-Dmode/xdim/ldim/iteration/offset\n\t\
-Eyr/mo/da/hr/mn/sc -Fformat -Iinfile -Nbuffersize -Ooutfile\n\t\
-Rwest/east/south/north -Smode/xdim/ldim/iteration\n\t\
-Tthreshold -V -H]";
	extern char *optarg;
	extern int optkind;
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
	char	ifile[MB_PATH_MAXLINE];
	void	*imbio_ptr = NULL;

	/* MBIO write control parameters */
	char	ofile[MB_PATH_MAXLINE];
	void	*ombio_ptr = NULL;

	/* mbio read and write values */
	void	*store_ptr;
	int	kind;
	int	nrecord = 0;
	int	nbathdata = 0;
	int	ndata = 0;
	char	comment[MB_COMMENT_MAXLINE];

	/* buffer handling parameters */
	void	*buff_ptr;
	int	n_buffer_max = MBFILTER_BUFFER_DEFAULT;
	int	nwant = MBFILTER_BUFFER_DEFAULT;
	int	nhold = 0;
	int	nhold_ping = 0;
	int	nbuff = 0;
	int	nload;
	int	ndump;
	int	ndumptot = 0;
	int	nexpect;
	struct mbfilter_ping_struct ping[MB_BUFFER_MAX];
	int	nping = 0;
	int	nping_start;
	int	nping_end;
	int	first = MB_YES;
	int	start, done;
	int	first_distance;
	double	save_time_d = 0.0;
	int	save_id = 0;

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	/* processing control variables */
	int	datakind = MBFILTER_SS;
	int	num_filters = 0;
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
	int	hipass_ndx, hipass_ndl;
	int	smooth_ndx, smooth_ndl;
	int	contrast_ndx, contrast_ndl;
	int	iteration;
	int	nstart = -1;
	int	nfinish = -1;

	double	*dataptr0, *dataptr1;
	char	*flagptr0, *flagptr1;
	double	ddis;
	int	ndatapts;
	int	ia,  ib;
	int	ja,  jb,  jbeg,  jend;
	int	i, j, k, ii, jj, kk;

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
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:F:f:HhI:i:M:m:N:n:O:o:R:r:S:s:T:t:Vv")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%d", &datakind);
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
			sscanf (optarg,"%d/%d/%d/%d",
				&contrast_mode, &contrast_xdim, 
				&contrast_ldim, &contrast_iter);
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%d/%d/%d/%d/%lf",
				&hipass_mode, &hipass_xdim, 
				&hipass_ldim, &hipass_iter, 
				&hipass_offset);
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
			sscanf (optarg,"%s", ifile);
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%d/%d", &nstart, &nfinish);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d", &n_buffer_max);
			if (n_buffer_max > MB_BUFFER_MAX
			    || n_buffer_max < 50)
			    n_buffer_max = MBFILTER_BUFFER_DEFAULT;
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			flag++;
			break;
		case 'R':
		case 'r':
			mb_get_bounds(optarg, bounds);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%d/%d/%d/%d",
				&smooth_mode, &smooth_xdim, 
				&smooth_ldim, &smooth_iter);
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

	/* set mode if not set */
	if (hipass_mode == MBFILTER_HIPASS_NONE
		&& smooth_mode == MBFILTER_SMOOTH_NONE
		&& contrast_mode == MBFILTER_CONTRAST_NONE)
		{
		smooth_mode = MBFILTER_SMOOTH_GAUSSIAN;
		smooth_xdim = 5;
		smooth_ldim = 3;
		smooth_iter = 3;
		}
		
	/* get number of filters applied */
	if (hipass_mode != MBFILTER_HIPASS_NONE) num_filters++;
	if (smooth_mode != MBFILTER_SMOOTH_NONE) num_filters++;
	if (contrast_mode != MBFILTER_CONTRAST_NONE) num_filters++;

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
		fprintf(stderr,"dbg2       input file:     %s\n",ifile);
		fprintf(stderr,"dbg2       output file:    %s\n",ofile);
		fprintf(stderr,"dbg2       datakind:       %d\n",datakind);
		fprintf(stderr,"dbg2       n_buffer_max:   %d\n",n_buffer_max);
		fprintf(stderr,"dbg2       hipass_mode:    %d\n",hipass_mode);
		fprintf(stderr,"dbg2       hipass_xdim:    %d\n",hipass_xdim);
		fprintf(stderr,"dbg2       hipass_ldim:    %d\n",hipass_ldim);
		fprintf(stderr,"dbg2       hipass_iter:    %d\n",hipass_iter);
		fprintf(stderr,"dbg2       hipass_offset:  %f\n",hipass_offset);
		fprintf(stderr,"dbg2       smooth_mode:    %d\n",smooth_mode);
		fprintf(stderr,"dbg2       smooth_xdim:    %d\n",smooth_xdim);
		fprintf(stderr,"dbg2       smooth_ldim:    %d\n",smooth_ldim);
		fprintf(stderr,"dbg2       smooth_iter:    %d\n",smooth_iter);
		fprintf(stderr,"dbg2       contrast_mode:    %d\n",contrast_mode);
		fprintf(stderr,"dbg2       contrast_xdim:    %d\n",contrast_xdim);
		fprintf(stderr,"dbg2       contrast_ldim:    %d\n",contrast_ldim);
		fprintf(stderr,"dbg2       contrast_iter:    %d\n",contrast_iter);
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
		mb_get_format(verbose,ifile,NULL,&format,&error);

	/* check for format with amplitude or sidescan data */
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

	/* output some information */
	if (verbose > 0)
		{
		if (datakind == MBFILTER_BATH)
			fprintf(stderr, "\nprocessing bathymetry data...\n");
		else if (datakind == MBFILTER_AMP)
			fprintf(stderr, "\nprocessing beam amplitude data...\n");
		else if (datakind == MBFILTER_SS)
			fprintf(stderr, "\nprocessing sidescan data...\n");
		if (hipass_mode == MBFILTER_HIPASS_MEAN)
			fprintf(stderr, "applying mean subtraction filter for hipass\n");
		else if (hipass_mode == MBFILTER_HIPASS_GAUSSIAN)
			fprintf(stderr, "applying gaussian mean subtraction filter for hipass\n");
		else if (hipass_mode == MBFILTER_HIPASS_MEDIAN)
			fprintf(stderr, "applying median subtraction filter for hipass\n");
		if (hipass_mode != MBFILTER_HIPASS_NONE)
			{
			fprintf(stderr, "  filter acrosstrack dimension: %d\n", hipass_xdim);
			fprintf(stderr, "  filter alongtrack dimension:  %d\n", hipass_ldim);
			fprintf(stderr, "  filter iterations:            %d\n", hipass_iter);
			fprintf(stderr, "  filter offset:                %f\n", hipass_offset);
			}
		if (smooth_mode == MBFILTER_SMOOTH_MEAN)
			fprintf(stderr, "applying mean filter for smoothing\n");
		else if (smooth_mode == MBFILTER_SMOOTH_GAUSSIAN)
			fprintf(stderr, "applying gaussian mean filter for smoothing\n");
		else if (smooth_mode == MBFILTER_SMOOTH_MEDIAN)
			fprintf(stderr, "applying median filter for smoothing\n");
		else if (smooth_mode == MBFILTER_SMOOTH_GRADIENT)
			fprintf(stderr, "applying inverse gradient filter for smoothing\n");
		if (smooth_mode == MBFILTER_SMOOTH_MEDIAN
			&& apply_threshold == MB_YES)
			{
			fprintf(stderr, "  filter low ratio threshold:   %f\n", threshold_lo);
			fprintf(stderr, "  filter high ratio threshold:  %f\n", threshold_hi);
			}
		if (smooth_mode != MBFILTER_SMOOTH_NONE)
			{
			fprintf(stderr, "  filter acrosstrack dimension: %d\n", smooth_xdim);
			fprintf(stderr, "  filter alongtrack dimension:  %d\n", smooth_ldim);
			fprintf(stderr, "  filter iterations:            %d\n", smooth_iter);
			}
		if (contrast_mode == MBFILTER_CONTRAST_EDGE)
			fprintf(stderr, "applying edge detection filter for contrast enhancement\n");
		else if (contrast_mode == MBFILTER_CONTRAST_GRADIENT)
			fprintf(stderr, "applying gradient subtraction filter for contrast enhancement\n");
		if (contrast_mode != MBFILTER_CONTRAST_NONE)
			{
			fprintf(stderr, "  filter acrosstrack dimension: %d\n", contrast_xdim);
			fprintf(stderr, "  filter alongtrack dimension:  %d\n", contrast_ldim);
			fprintf(stderr, "  filter iterations:            %d\n", contrast_iter);
			}
		}

	/* initialize reading the input swath sonar file */
	if ((status = mb_read_init(
		verbose,ifile,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize writing the output swath sonar file */
	if ((status = mb_write_init(
		verbose,ofile,format,&ombio_ptr,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	for (i=0;i<n_buffer_max;i++)
		{
		if (datakind == MBFILTER_BATH)
		    ndatapts = beams_bath;
		else if (datakind == MBFILTER_AMP)
		    ndatapts = beams_amp;
		else if (datakind == MBFILTER_SS)
		    ndatapts = pixels_ss;
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
		status = mb_malloc(verbose,beams_bath*sizeof(char),
			&ping[i].beamflag,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bath,&error);
		status = mb_malloc(verbose,beams_amp*sizeof(double),
			&ping[i].amp,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathacrosstrack,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathalongtrack,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(char),
			&ping[i].pixelflag,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ss,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ssacrosstrack,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ssalongtrack,&error);
		status = mb_malloc(verbose,ndatapts*sizeof(double),
			&ping[i].dataprocess,&error);
		status = mb_malloc(verbose,ndatapts*sizeof(double),
			    &ping[i].datasave,&error);		    
		}
		
	/* get ideal number of ping records to hold */
	nhold_ping = 1;
	if (hipass_mode != MBFILTER_HIPASS_NONE)
	    nhold_ping = MAX(nhold_ping, hipass_ldim / 2);
	if (smooth_mode != MBFILTER_SMOOTH_NONE)
	    nhold_ping = MAX(nhold_ping, smooth_ldim / 2);
	if (contrast_mode != MBFILTER_CONTRAST_NONE)
	    nhold_ping = MAX(nhold_ping, contrast_ldim / 2);
	
	/* allocate memory for weights */
	nweightmax = 2*MAX(hipass_xdim*hipass_ldim, 
	    MAX(smooth_xdim*smooth_ldim, contrast_xdim*contrast_ldim) );
	hipass_ndx = hipass_xdim/2;
	hipass_ndl = hipass_ldim/2;
	smooth_ndx = smooth_xdim/2;
	smooth_ndl = smooth_ldim/2;
	contrast_ndx = contrast_xdim/2;
	contrast_ndl = contrast_ldim/2;
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,nweightmax*sizeof(double),
				&weights,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,nweightmax*sizeof(double),
				&values,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,nweightmax*sizeof(double),
				&distances,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
		exit(error);
		}
		
	/* get acrosstrack dimensions to filter */
	if (nstart > nfinish 
	    || nstart < 0 
	    || nfinish < 0
	    || nstart > ndatapts - 1
	    || nfinish > ndatapts - 1)
	    {
	    nstart = 0;
	    nfinish = ndatapts - 1;
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
	sprintf(comment,"  Input file:         %s",ifile);
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

	/* initialize the buffer */
	status = mb_buffer_init(verbose,&buff_ptr,&error);

	/* read and write */
	done = MB_NO;
	first = MB_YES;
	nwant = n_buffer_max;
	if (verbose == 1) fprintf(stderr,"\n");
	while (!done)
		{
		/* load some data into the buffer */
		error = MB_ERROR_NO_ERROR;
		nexpect = nwant - nbuff;
		status = mb_buffer_load(verbose,buff_ptr,imbio_ptr,nwant,
				&nload,&nbuff,&error);
		nrecord += nload;

		/* give the statistics */
		if (verbose > 1) fprintf(stderr,"\n");
		if (verbose >= 1)
			{
			fprintf(stderr,"%d records loaded into buffer\n",nload);
			}

		/* check for done */
		if (nload < nexpect)
			{
			done = MB_YES;
			}

		/* extract data into ping arrays */
		ndata = 0;
		start = 0;
		jbeg = 0;
		jend = 0;
		status = MB_SUCCESS;
		while (status == MB_SUCCESS
			&& ndata < n_buffer_max)
			{
			/* get the data */
			status = mb_buffer_get_next_data(verbose,
				buff_ptr,imbio_ptr,start,&ping[ndata].id,
				ping[ndata].time_i,&ping[ndata].time_d,
				&ping[ndata].navlon,&ping[ndata].navlat,
				&ping[ndata].speed,&ping[ndata].heading,
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
				&error);
			if (status == MB_SUCCESS
			    && datakind == MBFILTER_SS)
			    {
			    for(i=0;i<ping[ndata].pixels_ss;i++)
				{
				if (ping[ndata].ss[i] > 0.0)
				    ping[ndata].pixelflag[i] 
					    = MB_FLAG_NONE;
				else
				    ping[ndata].pixelflag[i] 
					    = MB_FLAG_NULL;
				}
			    }
			if (status == MB_SUCCESS && first != MB_YES)
			    {
			    if (save_id == ping[ndata].id + ndumptot)
				jbeg = ndata + 1;
			    }
			if (status == MB_SUCCESS)
			    {
			    start = ping[ndata].id + 1;
			    ndata++;
			    }
			}
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		if (first == MB_YES)
			jbeg = 0;
		if (done == MB_YES)
			jend = ndata - 1;
		else if (ndata > nhold_ping + jbeg)
			{
			jend = ndata - 1 - nhold_ping;
			save_time_d = ping[jend].time_d;
			save_id = ping[jend].id + ndumptot;
			}
		else
			{
			jend = ndata - 1;
			save_time_d = ping[jend].time_d;
			save_id = ping[jend].id + ndumptot;
			}
		if (ndata > 0)
		    nbathdata += (jend - jbeg + 1);
		if (verbose >= 1)
			{
			fprintf(stderr,"%d survey records being processed\n",
				(jend - jbeg + 1));
			}
		if (first == MB_YES && nbathdata > 0)
			first = MB_NO;

		/* do hipass */
		iteration = 0;
		while (hipass_mode != MBFILTER_HIPASS_NONE
			&& iteration < hipass_iter)
		{
		/* increment iteration counter */
		iteration++;
		if (verbose > 0)
			fprintf(stderr, "doing hi-pass iteration %d...\n", iteration);

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
		  ja = j - hipass_ndl;
		  jb = j + hipass_ndl;
		  if (ja < 0) ja = 0;
		  if (jb >= ndata) jb = ndata - 1;

		  /* get data arrays and sizes to be used */
		  dataptr0 = ping[j].data_i_ptr;
		  flagptr0 = ping[j].flag_ptr;
		  ndatapts = ping[j].ndatapts;

		  /* loop over each value */
		  for (i=0;i<ndatapts;i++)
		    {
		    /* check if data is valid */
		    if (mb_beam_ok(flagptr0[i]))
		      {
		      /* get beginning and end values */
		      ia = i - hipass_ndx;
		      ib = i + hipass_ndx;
		      if (ia < 0) ia = 0;
		      if (ib >= ndatapts) ib = ndatapts - 1;

		      /* loop over surrounding pings and values */
		      nweight = 1;
		      values[0] = dataptr0[i];
		      distances[0] = 0.0;
		      for (jj=ja;jj<=jb;jj++)
		        for (ii=ia;ii<=ib;ii++)
		          {
			  dataptr1 = ping[jj].data_i_ptr;
			  flagptr1 = ping[jj].flag_ptr;
			  if (jj != j || ii != i 
			    && mb_beam_ok(flagptr1[ii]))
			    {
			    values[nweight] = dataptr1[ii];
			    ddis = 0.0;
			    if (hipass_ndx > 0)
				ddis += (ii - i)*(ii - i)/(hipass_ndx*hipass_ndx);
			    if (hipass_ndl > 0)
				ddis += (jj - j)*(jj - j)/(hipass_ndl*hipass_ndl);
			    distances[nweight] = sqrt(ddis);
			    nweight++;
			    }
			  }

		      /* get hipassed value */
		      if (hipass_mode == MBFILTER_HIPASS_MEAN)
		        hipass_mean(verbose, nweight, values, 
				weights, 
				&ping[j].data_f_ptr[i], &error);
		      else if (hipass_mode == MBFILTER_HIPASS_GAUSSIAN)
		        hipass_gaussian(verbose, nweight, values, 
				weights, distances, 
				&ping[j].data_f_ptr[i], &error);
		      else if (hipass_mode == MBFILTER_HIPASS_MEDIAN)
		        hipass_median(verbose, nweight, values, 
				weights, 
				&ping[j].data_f_ptr[i], &error);
		      }
		    }

		  /* print out progress */
/*		  if (verbose > 0)
		    fprintf(stderr, "done with ping %d of %d\n", 
			(j-jbeg+1), (jend-jbeg+1));*/
		  }
		  
		/* reset initial array and add offset 
		    if done with final iteration */
		if (iteration == hipass_iter)
		  for (j=0;j<ndata;j++)
		    for (i=0;i<ping[j].ndatapts;i++)
			ping[j].data_i_ptr[i] = ping[j].data_f_ptr[i]
				+ hipass_offset;
		else
		  for (j=0;j<ndata;j++)
		    for (i=0;i<ping[j].ndatapts;i++)
			ping[j].data_i_ptr[i] = ping[j].data_f_ptr[i];
		  
		/* save results if done with final iteration */
		if (iteration == hipass_iter)
		  for (j=jbeg;j<=jend;j++)
		    for (i=0;i<ping[j].ndatapts;i++)
			ping[j].datasave[i] = ping[j].data_i_ptr[i];
		}

		/* do smoothing */
		iteration = 0;
		while (smooth_mode != MBFILTER_SMOOTH_NONE
			&& iteration < smooth_iter)
		{
		/* increment iteration counter */
		iteration++;
		if (verbose > 0)
			fprintf(stderr, "doing lo-pass iteration %d...\n", iteration);

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
		  ja = j - smooth_ndl;
		  jb = j + smooth_ndl;
		  if (ja < 0) ja = 0;
		  if (jb >= ndata) jb = ndata - 1;

		  /* get data arrays and sizes to be used */
		  dataptr0 = ping[j].data_i_ptr;
		  flagptr0 = ping[j].flag_ptr;
		  ndatapts = ping[j].ndatapts;

		  /* loop over each value */
		  for (i=0;i<ndatapts;i++)
		    {
		    /* check if data is valid */
		    if (mb_beam_ok(flagptr0[i]))
		      {
		      /* get beginning and end values */
		      ia = i - smooth_ndx;
		      ib = i + smooth_ndx;
		      if (ia < 0) ia = 0;
		      if (ib >= ndatapts) ib = ndatapts - 1;

		      /* loop over surrounding pings and values */
		      nweight = 1;
		      values[0] = dataptr0[i];
		      distances[0] = 0.0;
		      for (jj=ja;jj<=jb;jj++)
		        for (ii=ia;ii<=ib;ii++)
		          {
			  dataptr1 = ping[jj].data_i_ptr;
			  flagptr1 = ping[jj].flag_ptr;
			  if (jj != j || ii != i 
			    && mb_beam_ok(flagptr1[ii]))
			    {
			    values[nweight] = dataptr1[ii];
			    ddis = 0.0;
			    if (smooth_ndx > 0)
				ddis += (ii - i)*(ii - i)/(smooth_ndx*smooth_ndx);
			    if (smooth_ndl > 0)
				ddis += (jj - j)*(jj - j)/(smooth_ndl*smooth_ndl);
			    distances[nweight] = sqrt(ddis);
			    nweight++;
			    }
			  }

		      /* get smoothed value */
		      if (smooth_mode == MBFILTER_SMOOTH_MEAN)
		        smooth_mean(verbose, nweight, values, weights, 
				&ping[j].data_f_ptr[i], &error);
		      else if (smooth_mode == MBFILTER_SMOOTH_GAUSSIAN)
		        smooth_gaussian(verbose, nweight, values, 
				weights, distances, 
				&ping[j].data_f_ptr[i], &error);
		      else if (smooth_mode == MBFILTER_SMOOTH_MEDIAN)
		        smooth_median(verbose, dataptr0[i], 
				apply_threshold, threshold_lo, threshold_hi,
				nweight, values, weights, 
				&ping[j].data_f_ptr[i], &error);
		      else if (smooth_mode == MBFILTER_SMOOTH_GRADIENT)
		        smooth_gradient(verbose, nweight, values, weights, 
				&ping[j].data_f_ptr[i], &error);
		      }
		    }

		  /* print out progress */
/*		  if (verbose > 0)
		    fprintf(stderr, "done with ping %d of %d\n", 
			(j-jbeg+1), (jend-jbeg+1));*/
		  }
		  
		/* reset initial array  */
		for (j=0;j<ndata;j++)
		    for (i=0;i<ping[j].ndatapts;i++)
			ping[j].data_i_ptr[i] = ping[j].data_f_ptr[i];
		  
		/* save results if done with final iteration */
		if (iteration == smooth_iter)
		  for (j=jbeg;j<=jend;j++)
		    for (i=0;i<ping[j].ndatapts;i++)
			ping[j].datasave[i] = ping[j].data_i_ptr[i];
		}

		/* do contrast enhancement */
		iteration = 0;
		while (contrast_mode != MBFILTER_CONTRAST_NONE
			&& iteration < contrast_iter)
		{
		/* increment iteration counter */
		iteration++;
		if (verbose > 0)
			fprintf(stderr, "doing contrast iteration %d...\n", iteration);

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
		  ja = j - contrast_ndl;
		  jb = j + contrast_ndl;
		  if (ja < 0) ja = 0;
		  if (jb >= ndata) jb = ndata - 1;

		  /* get data arrays and sizes to be used */
		  dataptr0 = ping[j].data_i_ptr;
		  flagptr0 = ping[j].flag_ptr;
		  ndatapts = ping[j].ndatapts;

		  /* loop over each value */
		  for (i=0;i<ndatapts;i++)
		    {
		    /* check if data is valid */
		    if (mb_beam_ok(flagptr0[i]))
		      {
		      /* get beginning and end values */
		      ia = i - contrast_ndx;
		      ib = i + contrast_ndx;
		      if (ia < 0) ia = 0;
		      if (ib >= ndatapts) ib = ndatapts - 1;

		      /* loop over surrounding pings and values */
		      nweight = 1;
		      values[0] = dataptr0[i];
		      distances[0] = 0.0;
		      for (jj=ja;jj<=jb;jj++)
		        for (ii=ia;ii<=ib;ii++)
		          {
			  dataptr1 = ping[jj].data_i_ptr;
			  flagptr1 = ping[jj].flag_ptr;
			  if (jj != j || ii != i 
			    && mb_beam_ok(flagptr1[ii]))
			    {
			    values[nweight] = dataptr1[ii];
			    ddis = 0.0;
			    if (contrast_ndx > 0)
				ddis += (ii - i)*(ii - i)/(contrast_ndx*contrast_ndx);
			    if (contrast_ndl > 0)
				ddis += (jj - j)*(jj - j)/(contrast_ndl*contrast_ndl);
			    distances[nweight] = sqrt(ddis);
			    nweight++;
			    }
			  }

		      /* get contrast enhanced value */
		      if (contrast_mode == MBFILTER_CONTRAST_EDGE)
		        contrast_edge(verbose, nweight, values, weights, 
				&ping[j].data_f_ptr[i], &error);
		      else if (contrast_mode == MBFILTER_CONTRAST_GRADIENT)
		        contrast_gradient(verbose, nweight, values, weights, 
				&ping[j].data_f_ptr[i], &error);
		      }
		    }

		  /* print out progress */
/* 		  if (verbose > 0)
		    fprintf(stderr, "done with ping %d of %d\n", 
			(j-jbeg+1), (jend-jbeg+1));*/
		  }
		  
		/* reset initial array  */
		for (j=0;j<ndata;j++)
		    for (i=0;i<ping[j].ndatapts;i++)
			ping[j].data_i_ptr[i] = ping[j].data_f_ptr[i];
		  
		/* save results if done with final iteration */
		if (iteration == contrast_iter)
		  for (j=jbeg;j<=jend;j++)
		    for (i=0;i<ping[j].ndatapts;i++)
			ping[j].datasave[i] = ping[j].data_i_ptr[i];
		}

		/* find number of pings to hold */
		if (done == MB_YES)
			nhold = 0;
		else if (jend <= jbeg)
			nhold = 0;
		else if (nhold_ping < jend)
			nhold = nbuff - ping[jend+1-nhold_ping].id;
		else if (ndata > 0)
			{
			nhold = nbuff - ping[0].id + 1;
			if (nhold > nbuff / 2)
				nhold = nbuff / 2;
			}
		else
			nhold = 0;
/*fprintf(stderr, "done:%d jbeg:%d jend:%d nbuff:%d nhold_ping:%d nhold:%d\n", 
done, jbeg, jend, nbuff, nhold_ping, nhold);*/

		/* reset pings to be dumped */
		if (ndata > 0)
		for (j=0;j<ndata-nhold_ping;j++)
		  {
		  if (datakind == MBFILTER_BATH)
		    status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[j].id,
				ping[j].time_i,ping[j].time_d,
				ping[j].navlon,ping[j].navlat,
				ping[j].speed,ping[j].heading,
				ping[j].beams_bath,
				ping[j].beams_amp,
				ping[j].pixels_ss,
				ping[j].beamflag,
				ping[j].datasave,
				ping[j].amp,
				ping[j].bathacrosstrack,
				ping[j].bathalongtrack,
				ping[j].ss,
				ping[j].ssacrosstrack,
				ping[j].ssalongtrack,
				comment,&error);
		  else if (datakind == MBFILTER_AMP)
		    status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[j].id,
				ping[j].time_i,ping[j].time_d,
				ping[j].navlon,ping[j].navlat,
				ping[j].speed,ping[j].heading,
				ping[j].beams_bath,
				ping[j].beams_amp,
				ping[j].pixels_ss,
				ping[j].beamflag,
				ping[j].bath,
				ping[j].datasave,
				ping[j].bathacrosstrack,
				ping[j].bathalongtrack,
				ping[j].ss,
				ping[j].ssacrosstrack,
				ping[j].ssalongtrack,
				comment,&error);
		  else if (datakind == MBFILTER_SS)
		    status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[j].id,
				ping[j].time_i,ping[j].time_d,
				ping[j].navlon,ping[j].navlat,
				ping[j].speed,ping[j].heading,
				ping[j].beams_bath,
				ping[j].beams_amp,
				ping[j].pixels_ss,
				ping[j].beamflag,
				ping[j].bath,
				ping[j].amp,
				ping[j].bathacrosstrack,
				ping[j].bathalongtrack,
				ping[j].datasave,
				ping[j].ssacrosstrack,
				ping[j].ssalongtrack,
				comment,&error);
		  }

		/* save processed data held in buffer */
		if (ndata > 0)
		for (j=0;j<=jend-(ndata-nhold_ping);j++)
		      {
		      jj = ndata - nhold_ping + j;
		      for (i=0;i<ping[jj].ndatapts;i++)
			ping[j].datasave[i] = ping[jj].datasave[i];
		      }

		/* dump data from the buffer */
		ndump = 0;
		if (nbuff > 0)
			{
			status = mb_buffer_dump(verbose,buff_ptr,ombio_ptr,
				nhold,&ndump,&nbuff,&error);
			ndumptot += ndump;
			}

		/* give the statistics */
		if (verbose >= 1)
			{
			fprintf(stderr,"%d records dumped from buffer\n",ndump);
			}
		}

	/* close the files */
	status = mb_buffer_close(verbose,&buff_ptr,imbio_ptr,&error);
	status = mb_close(verbose,&imbio_ptr,&error);
	status = mb_close(verbose,&ombio_ptr,&error);

	/* free the memory */
	for (j=0;j<n_buffer_max;j++)
		{
		mb_free(verbose,&ping[j].beamflag,&error); 
		mb_free(verbose,&ping[j].bath,&error); 
		mb_free(verbose,&ping[j].bathacrosstrack,&error); 
		mb_free(verbose,&ping[j].bathalongtrack,&error); 
		mb_free(verbose,&ping[j].amp,&error); 
		mb_free(verbose,&ping[j].ss,&error); 
		mb_free(verbose,&ping[j].ssacrosstrack,&error); 
		mb_free(verbose,&ping[j].ssalongtrack,&error); 
		mb_free(verbose,&ping[j].dataprocess,&error); 
		mb_free(verbose,&ping[j].datasave,&error); 
		}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d data records read and written\n",nrecord);
		fprintf(stderr,"%d survey data records processed\n",nbathdata);
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
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get mean */
	*hipass = 0.0;
	nn = 0;
	for (i=0;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			*hipass += val[i];
			nn++;
			}
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
	int	i, nn;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		fprintf(stderr,"dbg2       dis:             %d\n",dis);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f  dis[%d]: %f\n", 
				i, val[i], i, dis[i]);
		}

	/* get weights */
	*hipass = 0.0;
	wgtsum = 0.0;
	for (i=0;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			wgt[i] = exp(-dis[i]*dis[i]);
			wgtsum += wgt[i];
			}
		}

	if (wgtsum > 0.0)
		{
		/* get value */
		*hipass = 0.0;
		for (i=0;i<n;i++)
			{
			if (val[i] > 0.0)
				{
				*hipass += wgt[i]*val[i];
				}
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
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* start */
	*hipass = 0.0;

	/* sort values and get median value */
	if (n > 0)
		{
		qsort((char *)val,n,sizeof(double),mb_double_compare);
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
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get mean */
	*smooth = 0.0;
	nn = 0;
	for (i=0;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			*smooth += val[i];
			nn++;
			}
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
	int	i, nn;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		fprintf(stderr,"dbg2       dis:             %d\n",dis);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f  dis[%d]: %f\n", 
				i, val[i], i, dis[i]);
		}

	/* get weights */
	*smooth = 0.0;
	wgtsum = 0.0;
	for (i=0;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			wgt[i] = exp(-dis[i]*dis[i]);
			wgtsum += wgt[i];
			}
		}

	if (wgtsum > 0.0)
		{
		/* get value */
		*smooth = 0.0;
		for (i=0;i<n;i++)
			{
			if (val[i] > 0.0)
				{
				*smooth += wgt[i]*val[i];
				}
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
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* start */
	*smooth = 0.0;

	/* sort values and get median value */
	if (n > 0)
		{
		qsort((char *)val,n,sizeof(double),mb_double_compare);
		*smooth = val[n/2];
		}

	/* apply thresholding */
	if (apply_threshold == MB_YES)
		{
		ratio = original/(*smooth);
		if (ratio < threshold_hi
			&& ratio > threshold_lo)
			*smooth = original;
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
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
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
		if (val[i] > 0.0)
			{
			diff = fabs(val[i] - val[0]);
			if (diff < 0.01)
				diff = 0.01;
			wgt[i] = 1.0/diff;
			wgtsum += wgt[i];
			nn++;
			}
		}
	if (nn > 0)
		{
		*smooth = wgt[0]*val[0];
		for (i=1;i<n;i++)
			{
			if (val[i] > 0.0)
				{
				*smooth += 0.5*wgt[i]*val[i]/wgtsum;
				}
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
	int	i, ii, nn;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       grad:            %d\n",grad);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get gradients */
	gradsum = 0.0;
	edge = 0.0;
	for (i=0;i<n;i++)
	    {
	    grad[i] = 0.0;
	    if (val[i] > 0.0)
		{
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
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get weights */
	*result = 0.0;
	gradient = 0.0;
	nn = 0;
	for (i=1;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			gradient += (val[i] - val[0]) * (val[i] - val[0]);
			nn++;
			}
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
